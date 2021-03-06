#include "servermainwindow.h"
#include "./ui_servermainwindow.h"


ServerMainWindow::ServerMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ServerMainWindow)
{
    ui->setupUi(this);
    setWindowTitle("自主点餐系统服务端");


    // 连接数据库
    bool dbRet = connectDb();
    if (!dbRet)
    {
        QMessageBox::critical(this, "服务端启动失败", "无法连接数据库，请编辑目录下的config.json文件！");
    }


    // 创建Tcp服务器，并监听
    //establishTcp();
    _tcpServer = new QTcpServer;
    bool tcpRet = _tcpServer->listen(QHostAddress::Any,_tcpPort);

    if(!tcpRet)
    {
        QMessageBox::critical(this,"Tcp服务端未建立","未能建立Tcp服务端!");
    }
    else qDebug()<<"成功建立Tcp服务端";

    //当有客户端连接时，调用slotNewConnection方法
    connect(_tcpServer,&QTcpServer::newConnection,this,&ServerMainWindow::slotNewConnection);


    //收到sendMenuUpdateSiganl后调用slotSendMenuUpdateMessage槽函数
    connect(this,&ServerMainWindow::sendMenuUpdateSignal,this,&ServerMainWindow::slotSendMenuUpdateMessage);

    ui->tabWidget->setCurrentIndex(0);

    // Init Tab of Orders
    QVBoxLayout *layOrders = new QVBoxLayout(ui->tab_Orders);
    _table_Orders = new QTableWidget;
    layOrders->addWidget(_table_Orders);
    //订单号 桌号 价格 订单内容 订单备注
    _table_Orders->setColumnCount(6); //设置列数
    _table_Orders->setHorizontalHeaderLabels(QStringList()<<"状态"<<"订单号"<<"桌号"<<"价格"<<"订单内容"<<"订单备注"); //设置水平表头

    //设置列宽
    _table_Orders->setColumnWidth(0,120);
    _table_Orders->setColumnWidth(1,200);
    _table_Orders->setColumnWidth(2,90);
    _table_Orders->setColumnWidth(3,90);
    _table_Orders->setColumnWidth(4,300);
    _table_Orders->setColumnWidth(5,280);

    //让“订单内容”和订单备注随拉伸变化
    _table_Orders->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Stretch);
    _table_Orders->horizontalHeader()->setSectionResizeMode(5,QHeaderView::Stretch);

    //设置table属性
    _table_Orders->setFocusPolicy(Qt::NoFocus);  //失去焦点，让_table_Orders在未选中前不能删除
    _table_Orders->setSelectionBehavior(QAbstractItemView::SelectRows); //设置选择行行为，以行为单位
    _table_Orders->setSelectionMode(QAbstractItemView::SingleSelection); //设置选择模式，选择单行
    _table_Orders->setEditTriggers(QAbstractItemView::NoEditTriggers);  //设置为禁止编辑

    QHBoxLayout *layOrder_Btns = new QHBoxLayout;
    QPushButton *btnViewer = new QPushButton("查看该订单");
    QPushButton *btnReHandle = new QPushButton("重新处理该订单");
    QPushButton *btnHandle = new QPushButton("处理该订单");
    QPushButton *btnHistory = new QPushButton("查看历史订单");
    layOrder_Btns->addStretch(2);
    layOrder_Btns->addWidget(btnHistory);
    layOrder_Btns->addStretch(4);
    layOrder_Btns->addWidget(btnViewer);
    layOrder_Btns->addStretch(4);
    layOrder_Btns->addWidget(btnHandle);
    layOrder_Btns->addStretch(4);
    layOrder_Btns->addWidget(btnReHandle);
    layOrder_Btns->addStretch(2);
    layOrders->addLayout(layOrder_Btns);

    //绑定双击事件
    //connect(_table_Orders,SIGNAL(itemDoubleClicked(QTableWidgetItem *item)),this,SLOT(slotItemDoubleClicked(QTableWidgetItem *item)));
    //connect(_table_Orders,&QTableWidget::doubleClicked(const QModelIndex &index));
    //connect(_table_Orders,SIGNAL(SIGNAL(cellDoubleClicked(int , int ))),this,SLOT(slotCellDoubleClicked(int , int )));
    //TODO 绑定双击事件

    //绑定按钮
    connect(btnHistory,&QPushButton::clicked,this,&ServerMainWindow::slotBtnHistoryClicked);
    connect(btnReHandle,&QPushButton::clicked,this,&ServerMainWindow::slotBtnReHandleClicked);
    connect(btnHandle,&QPushButton::clicked,this,&ServerMainWindow::slotBtnHandleClicked);
    connect(btnViewer,&QPushButton::clicked,this,&ServerMainWindow::slotBtnViewerClicked);


    // Init Tab of Menu
    QVBoxLayout *layMenu = new QVBoxLayout(ui->tab_Menu);
    _view_Menu = new QTableView;
    layMenu->addWidget(_view_Menu);

    //设置_view_Menu表头
    _model->setHeaderData(0, Qt::Horizontal, "菜品编号");
    _model->setHeaderData(1, Qt::Horizontal, "菜品名称");
    _model->setHeaderData(2, Qt::Horizontal, "菜品种类");
    _model->setHeaderData(3, Qt::Horizontal, "菜品描述");
    _model->setHeaderData(4, Qt::Horizontal, "菜品价格");
    _model->setHeaderData(5, Qt::Horizontal, "菜品图片");
    _view_Menu->setModel(_model); //绑定model

    //设置_view_Menu属性
    _view_Menu->setColumnHidden(0,true); //隐藏ID列
    _view_Menu->setAlternatingRowColors(true); //行颜色变换
    _view_Menu->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);  //让“描述”随拉伸变化
    _view_Menu->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);  //让“图片”随拉伸变化
    _view_Menu->setColumnWidth(3, 250);
    _view_Menu->setColumnWidth(5, 250);
    _view_Menu->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置为选择单位为行
    _view_Menu->setSelectionMode(QAbstractItemView::SingleSelection); //设置选择一行
    _view_Menu->setEditTriggers(QAbstractItemView::NoEditTriggers);  //设置为只读

    QHBoxLayout *layMenu_btn = new QHBoxLayout(NULL);
    QPushButton *btnEdit = new QPushButton("修改菜品");
    QPushButton *btnAdd = new QPushButton("添加菜品");
    QPushButton *btnDel = new QPushButton("删除菜品");
    layMenu_btn->addWidget(btnEdit);
    layMenu_btn->addSpacing(15);
    layMenu_btn->addWidget(btnAdd);
    layMenu_btn->addSpacing(15);
    layMenu_btn->addWidget(btnDel);
    layMenu->addLayout(layMenu_btn);


    //绑定按钮信号与槽
    connect(btnEdit, &QPushButton::clicked, this, &ServerMainWindow::slotBtnEditClicked);
    connect(btnAdd, &QPushButton::clicked, this, &ServerMainWindow::slotBtnAddClicked);
    connect(btnDel, &QPushButton::clicked, this, &ServerMainWindow::slotBtnDelClicked);


    // Init Tab for Config
    QVBoxLayout *layConfig = new QVBoxLayout(ui->tab_Settings);

    QGroupBox *groupMySql = new QGroupBox("数据库设置");
    QVBoxLayout *layMySql = new QVBoxLayout(groupMySql);
    QLabel *lb_MySqlHost = new QLabel("MySql服务器地址：");
    le_MySqlHost = new QLineEdit;
    QLabel *lb_MySqlPort = new QLabel("MySql服务器端口：");
    le_MySqlPort = new QLineEdit;
    QLabel *lb_MySqlName = new QLabel("数据库名称：");
    le_MySqlName = new QLineEdit;
    QLabel *lb_MySqlUser = new QLabel("数据库用户名：");
    le_MySqlUser = new QLineEdit;
    QLabel *lb_MySqlPasswd = new QLabel("数据库密码：");
    le_MySqlPasswd = new QLineEdit;
    QHBoxLayout *layMySqlHost = new QHBoxLayout;
    layMySqlHost->addWidget(lb_MySqlHost);
    layMySqlHost->addWidget(le_MySqlHost);
    layMySqlHost->addStretch(1);
    QHBoxLayout *layMySqlPort = new QHBoxLayout;
    layMySqlPort->addWidget(lb_MySqlPort);
    layMySqlPort->addWidget(le_MySqlPort);
    layMySqlPort->addStretch(1);
    QHBoxLayout *layMySqlName = new QHBoxLayout;
    layMySqlName->addWidget(lb_MySqlName);
    layMySqlName->addWidget(le_MySqlName);
    layMySqlName->addStretch(1);
    QHBoxLayout *layMySqlUser = new QHBoxLayout;
    layMySqlUser->addWidget(lb_MySqlUser);
    layMySqlUser->addWidget(le_MySqlUser);
    layMySqlUser->addStretch(1);
    QHBoxLayout *layMySqlPasswd = new QHBoxLayout;
    layMySqlPasswd->addWidget(lb_MySqlPasswd);
    layMySqlPasswd->addWidget(le_MySqlPasswd);
    layMySqlPasswd->addStretch(1);

    layMySql->addLayout(layMySqlHost);
    layMySql->addLayout(layMySqlPort);
    layMySql->addLayout(layMySqlName);
    layMySql->addLayout(layMySqlUser);
    layMySql->addLayout(layMySqlPasswd);

    le_MySqlPasswd->setEchoMode(QLineEdit::Password);

    QGroupBox *groupHttp = new QGroupBox("图片HTTP服务器设置");
    QVBoxLayout *layHttp = new QVBoxLayout(groupHttp);
    QLabel *lb_HttpHost = new QLabel("HTTP服务端地址：");
    le_HttpHost = new QLineEdit;

    QHBoxLayout *layHttpHost = new QHBoxLayout;
    layHttpHost->addWidget(lb_HttpHost);
    layHttpHost->addWidget(le_HttpHost);
    layHttpHost->addStretch(1);

    layHttp->addLayout(layHttpHost);


    QGroupBox *groupTcp = new QGroupBox("TCP设置");
    QVBoxLayout *layTcp = new QVBoxLayout(groupTcp);
    QLabel *lb_TcpHost = new QLabel("TCP服务端IP：");
    le_TcpHost = new QLineEdit;
    QLabel *lb_TcpPort = new QLabel("TCP服务端端口：");
    le_TcpPort = new QLineEdit;

    QHBoxLayout *layTcpHost = new QHBoxLayout;
    layTcpHost->addWidget(lb_TcpHost);
    layTcpHost->addWidget(le_TcpHost);
    layTcpHost->addStretch(1);
    QHBoxLayout *layTcpPort = new QHBoxLayout;
    layTcpPort->addWidget(lb_TcpPort);
    layTcpPort->addWidget(le_TcpPort);
    layTcpPort->addStretch(8);

    layTcp->addLayout(layTcpHost);
    layTcp->addLayout(layTcpPort);


    QGroupBox *groupOrders = new QGroupBox("订单系统设置");
    QVBoxLayout *layConfigOrders = new QVBoxLayout(groupOrders);

    QHBoxLayout *layConfigOrdersClearShot = new QHBoxLayout;
    QLabel *lb_CLearShot = new QLabel("已处理订单消失等待时间(s)：");
    le_ClearShot = new QLineEdit;
    layConfigOrdersClearShot->addWidget(lb_CLearShot);
    layConfigOrdersClearShot->addWidget(le_ClearShot);
    layConfigOrdersClearShot->addStretch(1);

    layConfigOrders->addLayout(layConfigOrdersClearShot);



    QHBoxLayout *layConfigBtns = new QHBoxLayout;
    QPushButton *btnRevConfig = new QPushButton("恢复设置");
    QPushButton *btnUpdateConfig = new QPushButton("更新设置");
    layConfigBtns->addStretch(2);
    layConfigBtns->addWidget(btnRevConfig);
    layConfigBtns->addStretch(4);
    layConfigBtns->addWidget(btnUpdateConfig);
    layConfigBtns->addStretch(2);


    layConfig->addWidget(groupMySql);
    layConfig->addWidget(groupHttp);
    layConfig->addWidget(groupTcp);
    layConfig->addWidget(groupOrders);
    layConfig->addLayout(layConfigBtns);


    //初始化配置信息
    le_MySqlHost->setText(_dbHost);
    le_MySqlName->setText(_dbName);
    le_MySqlUser->setText(_dbUser);
    le_MySqlPort->setText(QString::number(_dbPort));
    le_MySqlPasswd->setText(_dbPasswd);

    le_HttpHost->setText(_picHost);

    le_TcpHost->setText(_tcpHost);
    le_TcpPort->setText(QString::number(_tcpPort));

    le_ClearShot->setText(QString::number(_clearShot));

    //绑定两按钮
    connect(btnUpdateConfig,&QPushButton::clicked,this,&ServerMainWindow::slotUpdateBtnClicked);
    connect(btnRevConfig,&QPushButton::clicked,this,&ServerMainWindow::slotRevBtnClicked);


    // Status Bar
    labelOrdersNoCount = new QLabel(tr("未处理订单数：%1").arg(QString::number(_OrdersNoCount)),this);
    labelOrdersCount = new QLabel(tr("总订单数：%1").arg(QString::number(_OrdersCount)),this);
    labelTime = new QLabel(getFormatTimeStamp("yyyy-MM-dd hh:mm:ss"),this);

    ui->statusbar->addPermanentWidget(labelOrdersNoCount);
    ui->statusbar->addPermanentWidget(labelOrdersCount);
    ui->statusbar->addPermanentWidget(labelTime);
    //启动计时器
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,[=](){
        labelTime->setText(getFormatTimeStamp("yyyy-MM-dd hh:mm:ss"));
    });
    timer->start(1000);

    //Sound
    sound = new QSound(":/Res/clock.wav");
}

ServerMainWindow::~ServerMainWindow()
{
    delete ui;
}

bool ServerMainWindow::connectDb()
{
    bool isOk = dbInfo.readFromFile("./config.json");

    if (!isOk)
    {
        qDebug() << "无法读取配置文件";
        QMessageBox::critical(this, "启动失败", "服务端启动失败，无法读取配置文件。");
        return false;
    }

    //赋值服务器信息
    _dbHost = dbInfo.getDbHost();
    _dbName = dbInfo.getDbName();
    _dbUser = dbInfo.getDbUser();
    _dbPasswd = dbInfo.getDbPasswd();
    _dbPort = dbInfo.getDbPort();

    //赋值图片HTTP服务器信息
    _picHost = dbInfo.getPicHost();

    //赋值tcp服务端信息
    _tcpHost = dbInfo.getTcpHost();
    _tcpPort = dbInfo.getTcpPort();

    //赋值订单服务端配置信息
    _clearShot = dbInfo.getClearShot();

    db.setHostName(_dbHost);
    db.setDatabaseName(_dbName);
    db.setUserName(_dbUser);
    db.setPassword(_dbPasswd);
    db.setPort(_dbPort);

    bool dbRet = db.open();

    if (!dbRet)
    {
        qDebug() << "数据库连接失败 " << db.lastError().text();
        return false;
    }
    else
        qDebug() << "数据库连接成功";

    //设置QSqlModel
    //menu表
    _model = new QSqlTableModel;
    _model->setTable("menu");
    _model->select();
    _model->setEditStrategy(QSqlTableModel::OnManualSubmit); //设置提交策略


    //menuType表
    _menuTypeModel = new QSqlTableModel;
    _menuTypeModel->setTable("menuType");
    _menuTypeModel->select();
    _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    //遍历menuType表初始化_menuTypeList

    for(int i=0; i<_menuTypeModel->rowCount(); i++)
    {
        QSqlRecord currentRecord = _menuTypeModel->record(i);
        _menuTypeList.append(currentRecord.value(1).toString());
    }

    qDebug()<<"_menuTypeList: "<<_menuTypeList;

    return true;
}

void ServerMainWindow::slotNewConnection()
{
    qDebug()<<"客户端连接";

    //判断是否有未处理的连接
    while(_tcpServer->hasPendingConnections())
    {
        qDebug()<<"处理链接";
        //调用nextPendingConnection去获得连接的socket
        QTcpSocket* currentSocket = _tcpServer->nextPendingConnection();
        _tcpSocket.append(currentSocket);
        //为新的scoket提供槽函数，来接收数据
        connect(currentSocket,&QTcpSocket::readyRead,this,&ServerMainWindow::slotReadyRead);
    }
}

void ServerMainWindow::slotReadyRead()
{
    //因为是多客户端，我没写判断是哪个socket发送的消息，所以我这里将整个socketlist遍历一下来判断是那个socket发送的消息
    for(int i=0; i<_tcpSocket.length();i++)
    {

        //接收数据，判断是否有数据，如果有，通过readAll函数获得所有数据
        qDebug()<<"Data arrived...";
        QByteArray buff=_tcpSocket[i]->readAll();
        qDebug()<<buff;

        if(buff.isEmpty()) continue;   //判断buff是否为空，若为空则说明不是这个客户端socket发送的


        //格式化处理订单消息
        //订单消息格式为"A03;127.5;[老八麻辣烫:5];3333"
        QString str = QString::fromUtf8(buff);  //先将订单消息转化为字符串
        //打印一下调信息，查看字符串内容及分区的信息
        qDebug()<<str;
        qDebug()<<str.section(";",0,0);
        qDebug()<<str.section(";",1,1);
        qDebug()<<str.section(";",2,2);
        qDebug()<<str.section(";",3,3);


        //先在订单的tableWidget中插入一行
        _table_Orders->setRowCount(_table_Orders->rowCount()+1);
        //qDebug()<<_table_Orders->rowCount();


        //赋值新插入的_table_Orders的新行
        //tableWidget一行是由多个QTableWidgetItem组成的，可以理解成一个单元格就是一个QTableWidgetItem
        _table_Orders->setItem(_table_Orders->rowCount()-1,0,new QTableWidgetItem("未处理"));  //每行第0列是订单状态

        //给当前订单构造一个订单号，以后我可能会把这个订单号构造的代码写成一个订单号类
        //订单号格式:2020061218344800002
        QString strOrderNum = QString::number(_OrdersCount);  //先获取一下当前的订单数，转化成字符串
        QString preZero; //因为要添加前置领，用preZero字符串变量存取一下
        qDebug()<<strOrderNum;
        qDebug()<<"size"<<strOrderNum.length();
        for(int i=1;i<=5-strOrderNum.length();i++) //我们设想当日的订单是00000-99999五位数，所以要根据当前订单数来循环添加订单号的前导零
        {
            preZero=preZero+'0';
        }
        //qDebug()<<preZero;
        strOrderNum=preZero+strOrderNum;  //把前导零和订单数拼接起来，构成订单后半部分
        //qDebug()<<strOrderNum;
        strOrderNum=getTimeStamp()+strOrderNum; //获取一下当前的时间戳，将时间戳作为订单号前半部分拼接起来
        qDebug()<<strOrderNum;
        _table_Orders->setItem(_table_Orders->rowCount()-1,1,new QTableWidgetItem(strOrderNum));  //每行第1列是订单号

        //格式化处理发送过来的订单信息，使用QString的section函数，以";"为分割标志，将订单信息各部分拆解
        QTableWidgetItem *itemTable = new QTableWidgetItem(str.section(";",0,0)); //桌号
        _table_Orders->setItem(_table_Orders->rowCount()-1,2,itemTable); //每行第2列是桌号

        QTableWidgetItem *itemPrice = new QTableWidgetItem(str.section(";",1,1));  //订单总价格
        _table_Orders->setItem(_table_Orders->rowCount()-1,3,itemPrice);  //每行第3列订单总价格

        QTableWidgetItem *itemOders = new QTableWidgetItem(str.section(";",2,2));  //订单内容
        _table_Orders->setItem(_table_Orders->rowCount()-1,4,itemOders);  //每行第4列是订单内容

        QTableWidgetItem *itemNote = new QTableWidgetItem(str.section(";",3,3));  //订单备注
        _table_Orders->setItem(_table_Orders->rowCount()-1,5,itemNote);  //每行第5列是订单备注

        //更新订单统计信息
        _OrdersCount++; //总订单数加1
        _OrdersNoCount++; //未处理订单数加1

        //更新状态栏
        ui->statusbar->showMessage("有新的订单！",2000);
        labelOrdersNoCount->setText(tr("未处理订单数：%1").arg(QString::number(_OrdersNoCount)));
        labelOrdersCount->setText(tr("总订单数：%1").arg(QString::number(_OrdersCount)));


        //Sound播放订单到达提醒声音
        sound->play();


    }
}

void ServerMainWindow::slotSendMenuUpdateMessage()
{
    qDebug()<<"slotSendMenuUpdateMessage";

    QTcpSocket *currentSocket;

    for(int i=0; i<_tcpSocket.size(); i++)  //遍历每一个客户端socket,发送信息
    {
        currentSocket = _tcpSocket.at(i);
        currentSocket->write(QString("[Menu Updated]").toUtf8());
    }

    qDebug()<<"SendMenuUpdateMessage Done";
}

void ServerMainWindow::slotBtnEditClicked()
{
    if (_view_Menu->currentIndex().row() == -1)  //检查是否选中某行
    {
        QMessageBox::critical(this, "编辑失败", "未选择任何行");
        return;
    }

    DialogEditRecord *dlg = new DialogEditRecord;  //初始化订单编辑对话框

    QSqlRecord record = _model->record(_view_Menu->currentIndex().row());  //利用QSqlRecord记录一下当前行的的record
    //将当前行的数据提取
    int dishId = record.value(0).toInt();
    QString dishName = record.value(1).toString();
    QString dishType = record.value(2).toString();
    QString dishInfo = record.value(3).toString();
    QString dishPrice = record.value(4).toString();
    QString dishPhoto =record.value(5).toString().mid(record.value(5).toString().lastIndexOf("/")+1,-1);
    qDebug()<<"dishPhoto:"<<dishPhoto;
    //赋值一下oldDishType
    oldDishType=dishType;

    dlg->setValue(dishId,dishName,dishType,dishInfo,dishPrice,dishPhoto,_picHost);  //传入值

    qDebug()<<"_picHost:"<<_picHost;
    dlg->show();


    //利用函数指针调用DialogEditRecord带参数的signalUpdate信号，和ServerMainWindow带参数的slotUpdate槽
    void (DialogEditRecord::*pSignalUpdate)(int, QString, QString, QString, QString, QString)=&DialogEditRecord::signalUpdate;
    void (ServerMainWindow::*pSlotUpdate)(int, QString, QString, QString, QString, QString)=&ServerMainWindow::slotUpdate;
    connect(dlg,pSignalUpdate,this,pSlotUpdate);

}

void ServerMainWindow::slotBtnAddClicked()
{
    DialogAddRecord *dlg = new DialogAddRecord;

    dlg->setUrl(_picHost);
    dlg->show();

    //利用函数指针调用DialogAddRecord带参数的signalSubmit信号，和ServerMainWindow带参数的slotSubmit槽
    void (DialogAddRecord::*pSignalSubmit)(QString, QString, QString, QString, QString)=&DialogAddRecord::signalSubmit;
    void (ServerMainWindow::*pSlotSubmit)(QString, QString, QString, QString, QString)=&ServerMainWindow::slotSubmit;
    connect(dlg,pSignalSubmit,this,pSlotSubmit);

}

void ServerMainWindow::slotBtnDelClicked()
{
    if (_view_Menu->currentIndex().row() == -1)
    {
        QMessageBox::critical(this, "删除失败", "未选择任何行");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除", "你确认要删除这个菜品吗？");
    if (ret == QMessageBox::Yes)
    {
        //先记录一下当前的部分信息
        QSqlRecord record = _model->record(_view_Menu->currentIndex().row());
        QString menuName = record.value(1).toString();
        QString fileName = record.value(5).toString().mid(record.value(5).toString().lastIndexOf("/"),-1);
        QString menuType = record.value(2).toString();

        bool delRet = _model->removeRow(_view_Menu->currentIndex().row());
        if (!delRet)
        {
            QMessageBox::critical(this, "删除失败", "删除菜品失败！");
            qDebug() << db.lastError().text();
            return;
        }
        bool submitRet = _model->submitAll();
        if (!submitRet)
        {
            QMessageBox::critical(this, "删除失败", "删除菜品失败！");
            qDebug() << db.lastError().text();
            return;
        }

        //删除本地和远程的图片
        QDir dir = QDir::currentPath()+"/Pic";

        if(dir.exists(menuName+".jpg"))
        {
            dir.remove(menuName+".jpg");
            qDebug()<<"Delete Menu: remove "+menuName+".jpg";

            //删除HTTP服务器上的文件
            HttpFileUpdate updateFile(fileName,_picHost+"/update.php");
            updateFile.update();
        }
        if(dir.exists(menuName+".jpeg"))
        {
            dir.remove(menuName+".jpeg");
            qDebug()<<"Delete Menu: remove "+menuName+".jpeg";

            //删除HTTP服务器上的文件
            HttpFileUpdate updateFile(fileName,_picHost+"/update.php");
            updateFile.update();
        }
        if(dir.exists(menuName+".png"))
        {
            dir.remove(menuName+".png");
            qDebug()<<"Delete Menu: remove "+menuName+".png";

            //删除HTTP服务器上的文件
            HttpFileUpdate updateFile(fileName,_picHost+"/update.php");
            updateFile.update();
        }


        //维护menuType表
        int index = _menuTypeList.indexOf(menuType);

        QSqlQuery query(db);
        if(index!=-1)
        {
            //index++;
            QSqlQuery indexQuery(db);
            qDebug()<<indexQuery.exec(tr("SELECT * FROM menuType WHERE TypeName='%1'").arg(menuType));
            indexQuery.next();
            QSqlRecord indexRecord = indexQuery.record();
            qDebug()<<indexRecord.value(1).toString();
            index=indexRecord.value(0).toInt();

            int menuTypeNum;
            qDebug()<<query.exec(tr("SELECT * FROM menuType WHERE ID=%1").arg(index));
            query.next();
            QSqlRecord record = query.record();
            qDebug()<<record.value(1).toString();
            menuTypeNum=record.value(2).toInt();
            qDebug()<<menuTypeNum;
            menuTypeNum--;
            qDebug()<<menuTypeNum;
            qDebug()<<query.exec(tr("UPDATE menuType SET NUM=%1 WHERE ID=%2").arg(menuTypeNum).arg(index));
        }

        //向客户端发送更新菜单消息
        emit sendMenuUpdateSignal();
    }

}

void ServerMainWindow::slotSubmit(QString dishName, QString dishType, QString dishInfo, QString dishPrice, QString dishPhoto)
{
    qDebug()<<"slotSubmit";
    //qDebug()<<dishName<<" "<<dishType<<" "<<dishInfo<<" "<<dishPrice<<" "<<dishPhoto;

    QSqlRecord record = _model->record();
    //插入record信息
    record.setValue(1,dishName);
    record.setValue(2,dishType);
    record.setValue(3,dishInfo);
    record.setValue(4,dishPrice.toDouble());
    record.setValue(5,_picHost+"/upload/"+dishPhoto);

    bool bRet = _model->insertRecord(-1,record); //插入record

    if(!bRet)
    {
        QMessageBox::critical(this,"添加失败","菜品添加失败！");
        qDebug()<<"add error "<<db.lastError().text();
        return;
    }
    else qDebug()<<"insert ok";


    _model->submitAll();


    //维护menuType表
    int index = _menuTypeList.indexOf(dishType);
    //QSqlRecord currentRecord;

    if(index==-1) //如果原本没有这个菜品种类
    {
        _menuTypeList.append(dishType); //增加到_menuTypeList里面


        QSqlQuery query(db);
        qDebug()<<query.exec(tr("INSERT INTO menuType (ID, TypeName, Num) VALUES(NULL,'%1',1)").arg(dishType));


        //给客户端发送更新菜单消息
        //注意一下发送消息的顺序,要先更显完菜品信息才发送更新信息
        emit sendMenuUpdateSignal();

    }
    else
    {
        //如果原来有这个菜品类
        //菜品数加1
        QSqlQuery indexQuery(db);
        qDebug()<<indexQuery.exec(tr("SELECT * FROM menuType WHERE TypeName='%1'").arg(dishType));
        indexQuery.next();
        QSqlRecord indexRecord = indexQuery.record();
        qDebug()<<indexRecord.value(1).toString();
        index=indexRecord.value(0).toInt();

        QSqlQuery query(db);
        qDebug()<<query.exec(tr("SELECT * FROM menuType WHERE ID=%1").arg(index));
        int menuTypeNum;
        query.next();
        QSqlRecord record = query.record();
        qDebug()<<record.value(1).toString();
        menuTypeNum=record.value(2).toInt();
        qDebug()<<menuTypeNum;
        menuTypeNum++;
        qDebug()<<menuTypeNum;
        qDebug()<<query.exec(tr("UPDATE menuType SET NUM=%1 WHERE ID=%2").arg(menuTypeNum).arg(index));


        //给客户端发送更新菜单消息
        //注意一下发送消息的顺序,要先更显完菜品信息才发送更新信息
        emit sendMenuUpdateSignal();
    }



}

void ServerMainWindow::slotUpdate(int dishId, QString dishName, QString dishType, QString dishInfo, QString dishPrice, QString dishPhoto)
{
    qDebug()<<"slotUpdate";
    //qDebug()<<dishId<<" "<<dishName<<" "<<dishType<<" "<<dishInfo<<" "<<dishPrice<<" "<<dishPhoto;
    //将更新信息插入record
    QSqlRecord record = _model->record(_view_Menu->currentIndex().row());
    record.setValue(1,dishName);
    record.setValue(2,dishType);
    record.setValue(3,dishInfo);
    record.setValue(4,dishPrice.toDouble());


    if((_picHost+"/upload/"+dishPhoto)!=record.value(5))  //检测图片是否更新
    {
        record.setValue(5,_picHost+"/upload/"+dishPhoto);
        qDebug()<<"图片改变";
    }

    bool bRet = _model->setRecord(_view_Menu->currentIndex().row(),record);

    if(!bRet)
    {
        QMessageBox::critical(this,"修改失败","菜品修改失败！");
        qDebug()<<"update error "<<db.lastError().text();
        return;
    }
    else qDebug()<<"update ok";


    _model->submitAll();


    //给客户端发送更新消息
    emit sendMenuUpdateSignal();


    //维护menuType表
    if(oldDishType != dishType)  //如果菜品种类改变
    {
        qDebug()<<"更换类型";
        int index = _menuTypeList.indexOf(oldDishType);
        if(index!=-1)
        {
            //将旧的菜品种类数减1
            QSqlQuery indexQuery(db);
            qDebug()<<indexQuery.exec(tr("SELECT * FROM menuType WHERE TypeName='%1'").arg(oldDishType));
            indexQuery.next();
            QSqlRecord indexRecord = indexQuery.record();
            qDebug()<<indexRecord.value(1).toString();
            index=indexRecord.value(0).toInt();

            QSqlQuery query(db);
            int menuTypeNum;
            qDebug()<<query.exec(tr("SELECT * FROM menuType WHERE ID=%1").arg(index));
            query.next();
            QSqlRecord record = query.record();
            qDebug()<<record.value(1).toString();
            menuTypeNum=record.value(2).toInt();
            qDebug()<<menuTypeNum;
            menuTypeNum--;
            qDebug()<<menuTypeNum;
            qDebug()<<query.exec(tr("UPDATE menuType SET NUM=%1 WHERE ID=%2").arg(menuTypeNum).arg(index));

        }
    }

    int index = _menuTypeList.indexOf(dishType);
    //QSqlRecord currentRecord;

    if(index==-1) //如果原本没有这个菜品种类
    {

        _menuTypeList.append(dishType); //增加到_menuTypeList里面

        QSqlQuery query(db);
        qDebug()<<query.exec(tr("INSERT INTO menuType (ID, TypeName, Num) VALUES(NULL,'%1',1)").arg(dishType));
    }
    else
    {
        //将菜品种类数加1
        QSqlQuery indexQuery(db);
        qDebug()<<indexQuery.exec(tr("SELECT * FROM menuType WHERE TypeName='%1'").arg(dishType));
        indexQuery.next();
        QSqlRecord indexRecord = indexQuery.record();
        qDebug()<<indexRecord.value(1).toString();
        index=indexRecord.value(0).toInt();

        QSqlQuery query(db);
        qDebug()<<query.exec(tr("SELECT * FROM menuType WHERE ID=%1").arg(index));
        int menuTypeNum;
        query.next();
        QSqlRecord record = query.record();
        qDebug()<<record.value(1).toString();
        menuTypeNum=record.value(2).toInt();
        qDebug()<<menuTypeNum;
        menuTypeNum++;
        qDebug()<<menuTypeNum;
        qDebug()<<query.exec(tr("UPDATE menuType SET NUM=%1 WHERE ID=%2").arg(menuTypeNum).arg(index));
    }


}

//重写退出事件
void ServerMainWindow::closeEvent(QCloseEvent *event)
{
    int ret = QMessageBox::question(this,"确认取消","您确认要退出服务端吗？");

    if(ret == QMessageBox::Yes)
    {
        event->accept();
    }
    else
        event->ignore();
}

void ServerMainWindow::slotUpdateBtnClicked()
{
    if(le_MySqlHost->text().isEmpty()||le_MySqlName->text().isEmpty()||le_MySqlUser->text().isEmpty()||le_MySqlPasswd->text().isEmpty()||le_TcpHost->text().isEmpty()||le_TcpPort->text().isEmpty()||le_ClearShot->text().isEmpty())
    {
        QMessageBox::critical(this,"错误","关键信息不完整！");
        return;
    }

    //更新信息
    _dbHost = le_MySqlHost->text();
    _dbName = le_MySqlName->text();
    _dbUser = le_MySqlUser->text();
    _dbPasswd = le_MySqlPasswd->text();
    _dbPort = le_MySqlPort->text().toInt();
    _picHost = le_HttpHost->text();
    _tcpHost = le_TcpHost->text();
    _tcpPort = le_TcpPort->text().toInt();
    _clearShot = le_ClearShot->text().toInt();

    WriteJson jsonConfig(_dbHost,_dbName,_dbUser,_dbPasswd,_dbPort,_picHost,_tcpHost,_tcpPort,_clearShot);  //写json配置文件
    if(!jsonConfig.writeToFile())
    {
        QMessageBox::critical(this,"错误","无法更新配置！");
    }

    /* 申请一个临时数据库，检测数据库配置信息和保存tcp信息 */
    QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QMYSQL");
    tmpdb.setHostName(_dbHost);
    tmpdb.setPort(_dbPort);
    tmpdb.setUserName(_dbUser);
    tmpdb.setDatabaseName(_dbName);
    tmpdb.setPassword(_dbPasswd);

    if(tmpdb.open())
    {
        QMessageBox::information(this,"服务器配置信息更新成功！","服务器配置信息更新成功，请重启服务端程序！");

        exit(0);
    }
    else
    {
        QMessageBox::critical(this,"无法连接MySql数据库","无法连接MySql数据库，请检查相关信息！");
        qDebug()<<tmpdb.lastError().text();
    }



}

void ServerMainWindow::slotRevBtnClicked()
{
    le_MySqlHost->setText(_dbHost);
    le_MySqlPort->setText(QString::number(_dbPort));
    le_MySqlName->setText(_dbName);
    le_MySqlUser->setText(_dbUser);
    le_MySqlPasswd->setText(_dbPasswd);

    le_TcpHost->setText(_tcpHost);
    le_TcpPort->setText(QString::number(_tcpPort));

    le_ClearShot->setText(QString::number(_clearShot));
}

void ServerMainWindow::slotBtnHistoryClicked()
{
    DialogHistoryViewer *dlg = new DialogHistoryViewer;

    dlg->exec();
}

void ServerMainWindow::slotBtnReHandleClicked()
{
    if(_table_Orders->currentIndex().row()==-1)  //判断是否选中一行
    {
        QMessageBox::critical(this,"处理失败","未选中任何行!");
        return;
    }

    if(_table_Orders->item(_table_Orders->currentIndex().row(),0)->text()=="未处理")  //判断该订单是否已处理
    {
        QMessageBox::critical(this,"处理失败","该订单未处理!");
        return;
    }

    int currentRow = _table_Orders->currentRow();  //获取一下当前行
    int ret = QMessageBox::question(this,"请求确认",tr("您确认要重新处理该订单吗？\n当前订单为:\n订单号: %1\n桌号: %2").arg(_table_Orders->item(currentRow,1)->text()).arg(_table_Orders->item(currentRow,2)->text()));
    if(ret==QMessageBox::Yes)
    {
        _table_Orders->setItem(currentRow,0,new QTableWidgetItem("未处理"));

        //更新订单统计信息
        _OrdersNoCount++; //未处理订单数减1

        //更新状态栏
        labelOrdersNoCount->setText(tr("未处理订单数：%1").arg(QString::number(_OrdersNoCount)));
    }

}

void ServerMainWindow::slotBtnHandleClicked()
{
    /* 订单处理
     * 先将该订单状态修改
     * 更新状态栏订单信息
     * 将该订单存入本地历史菜单数据库
     * 设定时间后删除该订单
    */

    if(_table_Orders->currentIndex().row()==-1) //判断是否已经选中一行
    {
        QMessageBox::critical(this,"处理失败","未选中任何行!");
        return;
    }

    if(_table_Orders->item(_table_Orders->currentIndex().row(),0)->text()!="未处理")  //判断该订单是否已经处理
    {
        QMessageBox::critical(this,"处理失败","该订单已经处理!");
        return;
    }

    int currentRow = _table_Orders->currentRow();  //获取一下当前订单的总行数

    /* 暴力做法，现在就先存一下该栏信息,后面遍历匹配删除 */
    QTableWidgetItem *currentItemOrderNum = _table_Orders->item(currentRow,1);

    int ret = QMessageBox::question(this,"请求确认",tr("您确认要处理该订单吗？\n当前订单为:\n订单号: %1\n桌号: %2").arg(_table_Orders->item(currentRow,1)->text()).arg(_table_Orders->item(currentRow,2)->text()));
    if(ret==QMessageBox::Yes)
    {
        _table_Orders->setItem(currentRow,0,new QTableWidgetItem(QIcon(":/Res/ok.ico"),"订单已处理")); //修改订单状态

        //更新订单统计信息
        _OrdersNoCount--; //未处理订单数减1

        //更新状态栏
        labelOrdersNoCount->setText(tr("未处理订单数：%1").arg(QString::number(_OrdersNoCount)));


        //添加该订单到本地数据库
        //建立数据连接
        QSqlDatabase tmpDb = QSqlDatabase::addDatabase("QSQLITE","tmpSqlite");
        //设置数据库文件名
        QString dbPath = QDir::currentPath()+"/"+"orders.db";
        tmpDb.setDatabaseName(dbPath);

        if(tmpDb.open())
        {
            qDebug()<<"sqlite数据库连接成功";
        }
        else qDebug()<<"sqlite数据库连接失败"<<tmpDb.lastError().text();

        QSqlQuery tmpQuery(tmpDb);
        //使用sql的insert语句插入数据
        bool rret = tmpQuery.exec(tr("INSERT INTO Orders VALUES(NULL,'%1','%2',%3,'%4','%5');").arg(_table_Orders->item(currentRow,1)->text()).arg(_table_Orders->item(currentRow,2)->text()).arg(_table_Orders->item(currentRow,3)->text()).arg(_table_Orders->item(currentRow,4)->text()).arg(_table_Orders->item(currentRow,5)->text()));
        //bool rret = tmpQuery.exec("INSERT INTO Orders VALUES(NULL,'2019053114563100002','A03',125,'[宫保鸡丁:1],[老八小汉堡:2],[扬州炒饭:2],[鱼香  丝:1]','希望能好吃');");
        qDebug()<<rret;
        //关闭本地数据库
        tmpDb.close();


        //QTimer 让已处理的订单定时消失
        QTimer *clearTimer = new QTimer(this);


        connect(clearTimer,&QTimer::timeout,[=](){
            //绑定QTimer的timeout信号，到规定时间开始处理
            for(int i=0;i<=_table_Orders->rowCount();i++) //遍历每一行
            {
                if(_table_Orders->item(i,1)==currentItemOrderNum)  //找到对应行，进行删除
                {
                    if(_table_Orders->item(i,0)->text()!="未处理")
                    {
                        _table_Orders->removeRow(i);
                        break;
                        qDebug()<<tr("清除第%1行").arg(QString::number(i));
                    }
                }
            }
        });
        clearTimer->setSingleShot(true); //设置为只执行一次
        clearTimer->start(_clearShot*1000);  //设置消失时间

    }
}

void ServerMainWindow::slotBtnViewerClicked()
{
    if(_table_Orders->currentIndex().row()==-1)  //判断一下是否选中某行订单信息，==-1表示未选中任何行
    {
        QMessageBox::critical(this,"查看失败","未选中任何行!");
        return;
    }

    DialogOrdersViewer *dlg = new DialogOrdersViewer;  //初始化订单信息查看对话框
    bool isHandled = true;  //isHandled储存当前订单处理状态
    if(_table_Orders->item(_table_Orders->currentIndex().row(),0)->text()=="未处理")
    {
        isHandled=false;
        qDebug()<<"未处理";
    }
    //传入值
    dlg->setData(isHandled,_table_Orders->item(_table_Orders->currentIndex().row(),1)->text(),_table_Orders->item(_table_Orders->currentIndex().row(),2)->text(),_table_Orders->item(_table_Orders->currentIndex().row(),3)->text(),_table_Orders->item(_table_Orders->currentIndex().row(),4)->text(),_table_Orders->item(_table_Orders->currentIndex().row(),5)->text());
    dlg->exec();

}


