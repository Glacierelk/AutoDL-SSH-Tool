#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QProcess *proxyProcess;
    QPushButton *pushButtonProxy;
    QPushButton *stopButton;

    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
