/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   communication.c                                                          */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/07 17:03:33 by Mateo                                    */
/*   Updated: 2019/02/05 12:22:00 by Mateo                                    */
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

// add tickers to queue
// data size will be allways multiple of sizeof(queue_t)
static uint16_t add_to_queue(const char *buff, const ssize_t readed,
														 const uint16_t data_size, const unsigned char offset,
														 tickers_t *tickers)
{
	static unsigned char leftover = 0; //when reaches sizeof(queue_t) is ok
	static char leftover_val[sizeof(queue_t)];
	ssize_t i;
	queue_t *tmp;

	i = 0;
	// if there are leftover in the buff complet them
	if (leftover)
		{
			// if still leftovers after this read retrun the size of the remminent
			if ((leftover = complete_reminent(buff, readed, leftover, leftover_val, &i)))
				return (data_size - i);
			// if not add the leftover to queue
			else
				{
					if (!(tmp = (queue_t *)malloc(sizeof(queue_t))))
						exit(EXIT_FAILURE);
					*tmp = *((queue_t *)leftover_val);
					tmp->next = tickers->queue;
					tickers->queue = tmp;
				}
		}
	// loop while we reach the end  of buff or we can get a complete queue_t node
	while ((i + sizeof(queue_t)) <= (size_t)readed &&
				 (i + sizeof(queue_t)) <= data_size)
		{
			// add the queue_t node to the queue
			if (!(tmp  = (queue_t *)malloc(sizeof(queue_t))))
				exit(2);
			*tmp = *((queue_t *)buff);
			tmp->next = tickers->queue;
			tickers->queue = tmp;
			i += sizeof(queue_t);
		}
	// set the leftover with the remminent of the buffer
	if (i < data_size && i < readed && readed == BUFF_SIZE - offset)
		leftover = complete_reminent(buff, readed, leftover, leftover_val, &i);
	// there is an error in the format
	else if (i < data_size && i < readed && readed != BUFF_SIZE - offset)
		{
			dprintf(2, "Error: in the data of the socket");
			exit(2);
		}
	return (data_size - i);
}

// send the list of tickers to the parser
static void send_tickers_parser(const client_t *cli, tickers_t *tickers)
{
	unsigned char len = 0;
	size_t buff_len = 0;
	size_t j;
	char *buff;
	char *value;

	// calculate  total len
	for (size_t i = 0; i < tickers->n_tuples; i++)
		buff_len += (*(tickers->tick_len))[i][PARSER_TICKERS_COL];
	//the len includes the null at end just but the size before the ticker
	buff_len = (buff_len + tickers->n_tuples + META_INFO_LEN);
	if (!(buff = (char *)malloc(sizeof(char) * buff_len)))
		{
			dprintf(2, "Error: in malloc send_tickers\n");
			exit(2);
		}
	//assign meta info (3 bytes)
	buff[0] = 0x03;
	*((unsigned short *)(buff + 1)) = tickers->n_tuples;
	j = META_INFO_LEN;
	// load all the data in the buff
	for (size_t i = 0; i < tickers->n_tuples; i++)
		{
			value = (*(tickers->tickers))[i][PARSER_TICKERS_COL];
			len = (*(tickers->tick_len))[i][PARSER_TICKERS_COL];
			buff[j++] = len;
			memcpy(buff + j, value, len);
			j += len;
		}
	//send it to the client
	send(cli->client_fd, buff, buff_len, 0x0);
	free (buff);
}

// add the tickers from the list
static void add_from_list(size_t *i, size_t *j, size_t batch, char *buff, tickers_t *tickers)
{
	char *value;
	unsigned char tick_len;

	// complete a batch with the list
	while (tickers->pos < tickers->n_tuples && *i < batch)
		{
			value = (*(tickers->tickers))[tickers->pos][VM_TICKERS_COL];
			tick_len = (*(tickers->tick_len))[tickers->pos][VM_TICKERS_COL];
			*((unsigned short *)(buff + *j)) = (unsigned short)tickers->pos;
			(*j) += 2;
			buff[(*j)++] = tick_len;
			printf ("%s (%d)\n", value, tick_len);
			memcpy(buff + *j, value, tick_len);
			(*j) += tick_len;
			(*i)++;
			tickers->pos++;
		}
}

// add the tickers from the queue
static void add_from_queue(size_t *i, size_t *j, size_t batch, char *buff, tickers_t *tickers)
{
	size_t queue_siz;
	char *value;
	unsigned char tick_len;
	queue_t *prev = NULL;
	queue_t *tmp = tickers->queue;
	queue_t *for_free = tickers->queue;

	// complete batch with queue until queue is empty
	while (*i < batch && tmp)
		{
			queue_siz = (tmp->end - tmp->start);
			if (queue_siz <= (batch - *i))
				{
					// load the tickers in the range specified by the queue node
					while(tmp->start < tmp->end)
						{
							value = (*(tickers->tickers))[tmp->start][VM_TICKERS_COL];
							tick_len = (*(tickers->tick_len))[tmp->start][VM_TICKERS_COL];
							*((unsigned short *)(buff + *j)) = (unsigned short)tmp->start;
							(*j) += 2;
							buff[(*j)++] = tick_len;
							memcpy(buff + *j, value, tick_len);
							(*j) += tick_len;
							(*i)++;
							(tmp->start)++;
						}
					// remove node from queue
					if (prev)
						prev->next = tmp->next;
					else
						tickers->queue = tmp->next;
					for_free = tmp;
					tmp = tmp->next;
					free(for_free);
				}
			else
				{
					prev = tmp;
					tmp = tmp->next;
				}
		}
}

// send the tickers to the virtual machines
static void send_tickers_vm(const client_t *cli, tickers_t *tickers)
{
	if (tickers->pos >= tickers->n_tuples && tickers->queue == NULL)
		return;
	printf("Sending batch of tickers\n");
	//i controls the position in the batch, j controls the position in the buffer
	size_t i, j;
	char buff[META_INFO_LEN + (BATCH_SIZE * (MAX_TICKER_LEN + 3))];

	//assign meta info (3 bytes)
	buff[0] = 0x04;
	i = 0;
	j = META_INFO_LEN;
	buff[j++] = BATCH_SIZE;
	// firs the queue is priorized then add tickers from the list
	add_from_queue(&i, &j, BATCH_SIZE, buff, tickers);
	add_from_list(&i, &j, BATCH_SIZE, buff, tickers);
	*((unsigned short *)(buff + 1)) = j - META_INFO_LEN;
	send(cli->client_fd, buff, j, 0x0);
	printf("Ticker list status: [%lu/%lu]\n", tickers->pos, tickers->n_tuples);
}

// send the id of the vm
static void send_vm_id(const client_t *cli)
{
	char buff[] = {SIG_VM_SEND_ID, 0x01, 0x00, cli->id};
	send(cli->client_fd, buff, META_INFO_LEN + 1,0x0);
}

// answer depending on the code received by the clients
void decode_data(const char *buff,
								 const ssize_t readed,
								 client_t *cli_head,
								 client_t *cli,
								 tickers_t *tickers,
								 uint64_t *flags)
{
	static uint8_t conn_code = 0xff;
	static uint16_t data_size = 0;
	unsigned char offset;
	unsigned char res;

	offset = 0;
	// if the code is still in use will be different to 0xFF
	// if is 0xFF a code will be assigned from the readed buffer
	if (conn_code == 0xFF)
		{
			if (readed <= META_INFO_LEN)
				return;
			conn_code = *((uint8_t *)buff);
			data_size = *((uint16_t *)(buff + 1));
			offset = 3;
		}
	// act depending on the code received (see protocol.txt)
	switch (conn_code)
		{
		case 0x00:
			printf("Machine started well\n");
			(*flags) |= (0x1 << cli->id);
			cli->is_vm = 1;
			send_vm_id(cli);
			send_tickers_vm(cli, tickers);
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
			send_tickers_parser(cli, tickers);
			conn_code = 0xFF;
			break;
		case 0x04:
			printf("Recived signal 0x04 this shall not happen\n");
			conn_code = 0xFF;
			break;
		case 0x05:
			printf("Machine has finished the batch\n");
			if (tickers->pos < tickers->n_tuples ||
					tickers->queue != NULL)
				send_tickers_vm(cli, tickers);
			else
				(*flags) &= ~(0x1 << cli->id);
			conn_code = 0xFF;
			break;
		case 0x06:
			printf("Recieved queue of tickers\n");
			data_size = add_to_queue(buff + offset, readed - offset, data_size, offset, tickers);
			if (!data_size)
				{
					while (cli_head)
						{
							if (cli_head->is_vm && !(*flags & (0x1 << cli_head->id)))
								send_tickers_vm(cli_head, tickers);
							cli_head = cli_head->next;
						}
					conn_code = 0xFF;
				}
			break;
		case 0x07:
			printf("Sending watching directories\n");
			res = VM_NB;
			send(cli->client_fd, &(res), sizeof(unsigned char), 0);
			conn_code = 0xFF;
			break;
		case 0x08:
			printf("Received SIG_SEND_VM_ID this shall not happen\n");
			conn_code = 0xFF;
			break;
		case SIG_PARS_RUN:
			*flags |= F_PARS_RUN;
			conn_code = 0xFF;
			break;
		case SIG_PARS_NO_RUN:
			*flags &= ~(F_PARS_RUN);
			conn_code = 0xFF;
			break;
		case SIG_END:
			res = 0x1;
			if (tickers->pos >= tickers->n_tuples &&
					tickers->queue == NULL &&
					!(*flags & F_PARS_RUN) &&
					!(*flags << N_FLAGS))
				{
					if (*flags & F_END)
						{
							printf("Ending the program\n");
							*flags |= F_END_2;
							res = 0x0;
						}
					else
						*flags |= F_END;
				}
			else
				*flags &= ~(F_END);
			send(cli->client_fd, &res, sizeof(unsigned char), 0);
			conn_code = 0xFF;
			break;
		default:
			printf("Connection code not recognized\n");
		}
}
