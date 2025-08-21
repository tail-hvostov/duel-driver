/*
Данный модуль предоставляет функции для ограничения доступа
устройств duel0-duel2 к некоторым операциям в зависимости
от каких-либо обстоятельств.
*/

#ifndef _DUEL_OPS_ACCESS_H_
#define _DUEL_OPS_ACCESS_H_

enum duel_device_ops {
    writing,
    raw_writing,
    str_reading
};

extern int duel_request_ops(u8 ops);
extern void duel_restore_ops(u8 ops);

#endif