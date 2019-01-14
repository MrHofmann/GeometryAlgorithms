#include "animationthread.h"

#include "algorithmbase.h"

AnimationThread::AnimationThread(AlgorithmBase *pAlgorithmBase)
    : QThread{}, _pAlgorithmBase{pAlgorithmBase}
{
}

void AnimationThread::run()
{
    if (_pAlgorithmBase)
        _pAlgorithmBase->runAlgorithm();
}
