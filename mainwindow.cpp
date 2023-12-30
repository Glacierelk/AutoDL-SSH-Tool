#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <windows.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , proxyProcess(nullptr)
{
    ui->setupUi(this);

    this->setWindowTitle("SSH 隧道工具");
    const QIcon icon = QIcon(":/favicon.png");
    this->setWindowIcon(icon);

    // 设置窗口背景色和固定大小
    this->setStyleSheet("background-color: #f5f5f5;");
    setFixedSize(500, 500);

    // 创建蓝色背景区域
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet("background-color: #723cef; color: white; border-radius: 10px;");
    headerFrame->setFixedHeight(80);

    // 添加标题标签
    QLabel *titleLabel = new QLabel("SSH 代理", headerFrame);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->addWidget(titleLabel);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    // 创建 LineEdit 和按钮
    QLabel *commandLabel = new QLabel("SSH 连接命令", this);
    commandLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin-top: 10px;");
    QLineEdit *lineEditCommand = new QLineEdit(this);
    lineEditCommand->setFixedHeight(30);
    lineEditCommand->setPlaceholderText("ssh -p [端口号] [用户名]@[主机名或IP地址]");

    QLabel *passwordLabel = new QLabel("SSH 密码", this);
    passwordLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin-top: 10px;");
    QLineEdit *lineEditPassword = new QLineEdit(this);
    lineEditPassword->setEchoMode(QLineEdit::Password);
    lineEditPassword->setFixedHeight(30);

    QLabel *portLabel = new QLabel("本地映射端口", this);
    portLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin-top: 10px;");
    QLineEdit *lineEditPort = new QLineEdit(this);
    lineEditPort->setText("8972");
    lineEditPort->setFixedHeight(30);

    QLabel *remotePortLabel = new QLabel("远程映射端口", this);
    remotePortLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin-top: 10px;");
    QLineEdit *lineEditRemotePort = new QLineEdit(this);
    lineEditRemotePort->setText("6006");
    lineEditRemotePort->setFixedHeight(30);

    pushButtonProxy = new QPushButton("代理", this);
    QLabel *localAddressLabel = new QLabel(this);

    // 创建停止代理按钮
    stopButton = new QPushButton("停止代理", this);
    stopButton->setStyleSheet("background-color: #FF0000; border-radius: 5px; color: white; padding: 10px; border: none;");

    // 垂直布局
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(headerFrame);
    layout->addWidget(commandLabel);
    layout->addWidget(lineEditCommand);
    layout->addWidget(passwordLabel);
    layout->addWidget(lineEditPassword);
    layout->addWidget(portLabel);
    layout->addWidget(lineEditPort);
    layout->addWidget(remotePortLabel);
    layout->addWidget(lineEditRemotePort);
    layout->addWidget(pushButtonProxy);
    layout->addWidget(stopButton);
    layout->addWidget(localAddressLabel);

    // 设置标签对齐方式
    commandLabel->setAlignment(Qt::AlignLeft);
    passwordLabel->setAlignment(Qt::AlignLeft);
    portLabel->setAlignment(Qt::AlignLeft);

    // 将布局应用到主窗口中
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // 设置按钮样式
    pushButtonProxy->setStyleSheet("QPushButton {"
                                   "background-color: #4CAF50;"
                                   "border-radius: 5px;"
                                   "color: white;"
                                   "padding: 10px;"
                                   "border: none;"
                                   "}"
                                   "QPushButton:hover {"
                                   "background-color: #45a049;"
                                   "}");

    // 设置链接文本样式
    localAddressLabel->setTextFormat(Qt::RichText);
    localAddressLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    localAddressLabel->setOpenExternalLinks(true);
    localAddressLabel->setStyleSheet("color: #007ACC; text-decoration: none;");

    // 隐藏停止代理按钮（初始时不可见）
    stopButton->hide();

    // 连接停止代理按钮的点击事件
    connect(stopButton, &QPushButton::clicked, this, [this, localAddressLabel]() {
        if (proxyProcess && proxyProcess->state() == QProcess::Running) {
            proxyProcess->kill();  // 终止代理进程
            system("taskkill /F /IM plink.exe");
            proxyProcess = nullptr;
            localAddressLabel->setText("");
            stopButton->hide();
            pushButtonProxy->show();
        }
    });

    // 连接代理按钮点击事件
    connect(pushButtonProxy, &QPushButton::clicked, this,
            [this, lineEditCommand, lineEditPassword, lineEditPort, localAddressLabel, lineEditRemotePort]() {
        QString command = lineEditCommand->text();
        QString password = lineEditPassword->text();
        QString port = lineEditPort->text();
        QString remotePort = lineEditRemotePort->text();

        if (command.isEmpty() || password.isEmpty() || port.isEmpty()) {
            QMessageBox::warning(this, "警告", "请填写完整信息");
            return;
        }

        QString proxyCommand = "/c plink -C -N -L " + port + ":127.0.0.1:" + remotePort + " -P " + command.split(" ")[2]
                               + " " + command.split(" ")[3] + " -pw " + password + " -no-antispoof -batch";

        // std::cout << proxyCommand.toStdString() << "\n";

        if (!proxyProcess || proxyProcess->state() != QProcess::Running) {
            QString fullCommand = "cmd.exe ";
            QStringList arguments;
            arguments << proxyCommand;

            proxyProcess = new QProcess(this);
            proxyProcess->start(fullCommand, arguments);  // 启动代理进程

            if (!proxyProcess || proxyProcess->state() != QProcess::Running) {
                return;
            }

            stopButton->show();  // 显示停止代理按钮
            pushButtonProxy->hide();

            QString localAddress = "http://127.0.0.1:" + port; // 这里根据实际情况设置本地地址

            localAddressLabel->setText("<a href=\"" + localAddress + "\">" + localAddress + "</a>");
            localAddressLabel->setOpenExternalLinks(true);
        }
    });
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // 关闭应用时终止代理进程
    if (proxyProcess && proxyProcess->state() == QProcess::Running) {
        proxyProcess->kill();
        system("taskkill /F /IM plink.exe");
    }
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow()
{
    if (proxyProcess) {
        proxyProcess->kill();  // 确保代理进程终止
        proxyProcess->waitForFinished(3000);
        delete proxyProcess;
    }
    delete ui;
}
