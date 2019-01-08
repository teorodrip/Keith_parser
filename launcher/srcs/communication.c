/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   communication.c                                                          */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/07 17:03:33 by Mateo                                    */
/*   Updated: 2019/01/08 12:13:02 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.h"

static unsigned char complete_reminent(const char *buff, const ssize_t readed,
																			 unsigned char reminent, queue_t *tmp)
{
	ssize_t i;

	i = 0;
	while (reminent < readed && reminent < sizeof(uint32_t))
		{
			((uint8_t *)(&tmp->start))[i] = buff[reminent++];
			i++;
		}
	i = 0;
	while (reminent < readed && reminent < (sizeof(uint32_t) * 2))
		{
			((uint8_t *)(&tmp->end))[i] = buff[reminent++];
			i++;
		}
	tmp->next = queue_g;
	queue_g = tmp;
	return (reminent % (sizeof(uint32_t) * 2));
}

// data size will be allways multiple of sizeof(uint32_t) * 2
static uint16_t add_to_queue(const char *buff, const ssize_t readed, const uint16_t data_size)
{
	static unsigned char reminent = 0;
	ssize_t i;
	queue_t *tmp;

	i = 0;
	if (reminent)
		{
			i += reminent;
			reminent = complete_reminent(buff, readed, reminent, queue_g);
		}
	while ((i + (sizeof(uint32_t) * 2)) < readed &&
				 (i + (sizeof(uint32_t) * 2)) < data_size)
		{
			if (!(tmp  = (queue_t *)malloc(sizeof(queue_t))))
				exit(2);
			tmp->start = *((uint32_t *)buff + i);
			tmp->end = *((uint32_t *)(buff + i + sizeof(uint32_t)));
			tmp->next = queue_g;
			queue_g = tmp;
			i += sizeof(uint32_t) * 2;
		}
	if (i < data_size && i < readed && readed == BUFF_SIZE)
		{
			if (!(tmp  = (queue_t *)malloc(sizeof(queue_t))))
				exit(2);
			reminent = complete_reminent(buff + i, readed - i, reminent, tmp);
		}
	else if (readed != BUFF_SIZE)
		{
			dprintf(2, "Error: in the data of the socket");
			exit(2);
		}
	return (data_size - i);
}

void decode_data(const char *buff, const ssize_t readed)
{
	static uint8_t conn_code = 0xff;
	static uint16_t data_size = 0;
	unsigned char offset;

	offset = 0;
	(void)offset;
	(void)readed;
	if (conn_code == 0xFF)
		{
			if (readed <= META_INFO_LEN)
				return;
			conn_code = *((uint8_t *)buff);
			data_size = *((uint16_t *)(buff + 1));
			printf("Readed code: %x (%u bytes)\n", conn_code, data_size);
			offset = 3;
		}
	switch (conn_code)
		{
		case 0x00:
			printf("Machine started well\n");
			conn_code = 0xFF;
			break;
		case 0x01:
			printf("Machine shutdown\n");
			conn_code = 0xFF;
			break;
		case 0x02:
			printf("Machine reboot\n");
			conn_code = 0xFF;
			break;
		case 0x03:
			printf("Sending list of all tickers\n");
			conn_code = 0xFF;
			break;
		case 0x04:
			printf("Sending batch of tickers\n");
			conn_code = 0xFF;
			break;
		case 0x05:
			printf("Machine has finished the batch\n");
			conn_code = 0xFF;
			break;
		default:
			printf("Connection code not recognized\n");
		}
}
