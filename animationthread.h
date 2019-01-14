#ifndef ANIMATIONTHREAD_H
#define ANIMATIONTHREAD_H

#include <QThread>

class AlgorithmBase;

class AnimationThread : public QThread
{
public:
    ///
    /// \brief AnimationThread
    /// \param pAlgorithmBase - algoritam koji se izvrsava unutar thread-a
    ///
    AnimationThread(AlgorithmBase *pAlgorithmBase);

    ///
    /// \brief run - metod koji se izvrsava kada se thread pokrene
    ///
    virtual void run();
private:
    AlgorithmBase *_pAlgorithmBase;
};

#endif // ANIMATIONTHREAD_H
