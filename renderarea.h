#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QtGui>

class AlgorithmBase;

class RenderArea : public QWidget
{
    Q_OBJECT
public:
    explicit RenderArea(QWidget *parent = nullptr);
    void setPAlgorithmBase(AlgorithmBase *pAlgorithmBase);

protected:
    ///
    /// \brief paintEvent -
    ///         Funkcija (preciznije event handler) u kojoj je implementirano
    ///         iscrtavanje po nasoj oblasti za crtanje.
    ///
    void paintEvent(QPaintEvent*) override;
private:
    AlgorithmBase *_pAlgorithmBase;
};

#endif // RENDERAREA_H
