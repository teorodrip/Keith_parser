/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   protocol.h                                                               */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/23 10:52:35 by Mateo                                    */
/*   Updated: 2019/01/23 11:00:56 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROT_H
#define PROT_H

#define SIG_VM_START 0x0
#define SIG_VM_POWEROFF 0x1
#define SIG_VM_REBOOT 0x2
#define SIG_SEND_TICK 0x3
#define SIG_SEND_BATCH 0x4
#define SIG_VM_FINISH 0x5
#define SIG_GET_QUEUE 0x6
#define SIG_SEND_NUM_VM 0x7
#define SIG_VM_SEND_ID 0x8
#define SIG_PARS_RUN 0x9
#define SIG_PARS_NO_RUN 0xA
#define SIG_END 0xB
#define SIG_NULL 0xFF

#endif
