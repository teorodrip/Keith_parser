/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   communication.c                                                          */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/07 17:03:33 by Mateo                                    */
/*   Updated: 2019/01/10 17:13:17 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/launcher.h"
static unsigned char complete_reminent(const char *buff, const ssize_t readed,
																			 unsigned char leftover, char *leftover_val, ssize_t *i)
{
	while (*i < readed && leftover < sizeof(queue_t))
		{
			leftover_val[leftover] = buff[*i];
			leftover++;
			(*i)++;
		}
	return (leftover % sizeof(queue_t));
}

// data size will be allways multiple of sizeof(queue_t)
static uint16_t add_to_queue(const char *buff, const ssize_t readed,
														 const uint16_t data_size, const unsigned char offset)
{
	static unsigned char leftover = 0; //when reaches sizeof(queue_t) is ok
	static char leftover_val[sizeof(queue_t)];
	ssize_t i;
	queue_t *tmp;

	i = 0;
	if (leftover)
		{
			if ((leftover = complete_reminent(buff, readed, leftover, leftover_val, &i)))
				return (data_size - i);
			else
				{
					if (!(tmp = (queue_t *)malloc(sizeof(queue_t))))
						exit(EXIT_FAILURE);
					*tmp = *((queue_t *)leftover_val);
					tmp->next = queue_g;
					queue_g = tmp;
				}
		}
	while ((i + sizeof(queue_t)) <= (size_t)readed &&
				 (i + sizeof(queue_t)) <= data_size)
		{
			if (!(tmp  = (queue_t *)malloc(sizeof(queue_t))))
				exit(2);
			*tmp = *((queue_t *)leftover_val);
			tmp->next = queue_g;
			queue_g = tmp;
			i += sizeof(queue_t);
		}
	if (i < data_size && i < readed && readed == BUFF_SIZE - offset)
		leftover = complete_reminent(buff, readed, leftover, leftover_val, &i);
	else if (i < data_size && i < readed && readed != BUFF_SIZE - offset)
		{
			dprintf(2, "Error: in the data of the socket");
			exit(2);
		}
	return (data_size - i);
}

void decode_data(const char *buff, const ssize_t readed,
								 const client_t *cli, const tickers_t *tickers)
{
	static uint8_t conn_code = 0xff;
	static uint16_t data_size = 0;
	unsigned char offset;

	/* write(1, buff, readed); */
	/* write(1, "\n", 1); */
	offset = 0;
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
			send(cli->client_fd, &(tickers->n_tuples), sizeof(size_t), 0);
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
		case 0x06:
			printf("Machine has finished the batch\n");
			data_size = add_to_queue(buff + offset, readed - offset, data_size, offset);
			if (!data_size)
				conn_code = 0xFF;
			break;
		case 0x07:
			printf("Sending watching directories\n");
			unsigned char n = VM_NB;
			send(cli->client_fd, &(n), sizeof(unsigned char), 0);
			conn_code = 0xFF;
			break;
		default:
			printf("Connection code not recognized\n");
		}
}
