/*
Данный модуль предоставляет функции для ограничения доступа
устройств duel0-duel2 к некоторым операциям в зависимости
от каких-либо обстоятельств.
*/

#ifndef _DUEL_OPS_ACCESS_H_
#define _DUEL_OPS_ACCESS_H_

#define DUEL_OP_WRITING 0x1
#define DUEL_OP_RAW_WRITING 0x2
#define DUEL_OP_STR_READING 0x2

#define DUEL_STAT_STR_CONSISTENCY 0x1

//Может вернуть предыдущее состояние ops_stats, если второй аргумент не NULL.
extern int duel_request_ops(unsigned long ops, unsigned long* stats);
extern void duel_restore_ops(unsigned long ops);

#endif