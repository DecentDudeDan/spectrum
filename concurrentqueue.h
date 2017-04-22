#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <QMutex>
#include <QQueue>
#include "qqueue.h"
#include <complex>

class ConcurrentQueue;

class ConcurrentQueue
{
private:
QQueue<std::complex<double>> points;
QMutex mutex;

public:

ConcurrentQueue();
void enqueue(std::complex<double> value);
std::complex<double> dequeue();
bool isEmpty();
int size();
void unlock();
void clear();

};

#endif // CONCURRENTQUEUE_H
