#include "dialogcartview.h"

DialogCartView::DialogCartView(const QList<CartItem>&cartlist, QWidget *parent)
    :QDialog(parent),_cartList(cartlist)
{

    this->setWindowIcon(QIcon(":/Res/shoppingcart.png"));
    this->setWindowTitle("查看购物车");
    this->setFixedSize(QSize(600,600));


    //获取购物车总价格和总件数_cartPrice、_cartNum的值
    for(int i=0;i<_cartList.size();i++)
    {
        _cartPrice+=_cartList.at(i).getSum();
    }

    for(int i=0;i<_cartList.size();i++)
    {
        _cartNum+=_cartList.at(i).getNum();
    }

    /* 界面初始化 */
    QVBoxLayout *lay = new QVBoxLayout(this);

    _cartTable = new QTableWidget;

    lb_cartNum = new QLabel("购物车菜品总数：");
    lb_cartNumContent = new QLabel(QString::number(_cartNum));

    lb_cartPrice = new QLabel("购物车总价：");
    lb_cartPriceContent = new QLabel(QString::number(_cartPrice));

    QHBoxLayout *layNum = new QHBoxLayout;
    layNum->addWidget(lb_cartNum);
    layNum->addWidget(lb_cartNumContent);

    QHBoxLayout *layPrice = new QHBoxLayout;
    layPrice->addWidget(lb_cartPrice);
    layPrice->addWidget(lb_cartPriceContent);

    QHBoxLayout *layButtons = new QHBoxLayout;
    QPushButton *btn_clear = new QPushButton("清空购物车");
    QPushButton *btn_checkout = new QPushButton("结算");
    btn_clear->setIcon(QIcon(":/Res/delete.png")); btn_clear->setIconSize(QSize(40,40));
    btn_checkout->setIcon(QIcon(":/Res/checkout.png")); btn_checkout->setIconSize(QSize(40,40));
    layButtons->addStretch(1);
    layButtons->addWidget(btn_clear);
    layButtons->addStretch(2);
    layButtons->addWidget(btn_checkout);
    layButtons->addStretch(1);

    lay->addWidget(_cartTable);
    lay->addLayout(layNum);
    lay->addLayout(layPrice);

    lay->addLayout(layButtons);


    //配置tableWidget内容
    QStringList tableHeaders;
    tableHeaders<<"菜品名"<<"数量"<<"总价格";
    _cartTable->setColumnCount(3);
    _cartTable->setHorizontalHeaderLabels(tableHeaders);
    //让三列随拉伸变化
    _cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _cartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    _cartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    //增加tableWidget内容
    for(int i=0;i<_cartList.size();i++)
    {
        QTableWidgetItem *currentItemName = new QTableWidgetItem;
        QTableWidgetItem *currentItemNum = new QTableWidgetItem;
        QTableWidgetItem *currentItemPrice = new QTableWidgetItem;
        currentItemName->setText(_cartList.at(i).getItemName());
        currentItemName->setFlags(currentItemName->flags() &= (~Qt::ItemIsEditable));
        currentItemNum->setText(QString::number(_cartList.at(i).getNum()));
        currentItemPrice->setText(QString::number(_cartList.at(i).getSum()));
        currentItemPrice->setFlags(currentItemPrice->flags() &= (~Qt::ItemIsEditable));

        //插入新行
        _cartTable->setRowCount(_cartTable->rowCount()+1);
        //为新行赋值
        int currentRow = _cartTable->rowCount()-1;
        _cartTable->setItem(currentRow,0,currentItemName);
        _cartTable->setItem(currentRow,1,currentItemNum);
        _cartTable->setItem(currentRow,2,currentItemPrice);
    }

    //添加代理
    _cartTable->setItemDelegateForColumn(1,new MySpinBoxDelegate);

    //关联按钮事件
    connect(btn_clear,&QPushButton::clicked,this,&DialogCartView::btnClearClicked);
    connect(btn_checkout,&QPushButton::clicked,this,&DialogCartView::btnCheckOutBtnClicked);
    void (QTableWidget::*pSignalItemChanged)(QTableWidgetItem *) = &QTableWidget::itemChanged;
    void (DialogCartView::*pSlotCartChanged)(QTableWidgetItem *) = &DialogCartView::slotCartChanged;
    connect(_cartTable,pSignalItemChanged,this,pSlotCartChanged);
}

void DialogCartView::btnClearClicked()
{
    int ret = QMessageBox::question(this,"请求确认","您确认要清空购物车吗？");

    if(ret == QMessageBox::Yes)
    {
        _cartList.clear(); //购物车列表清空

        for(int i=_cartTable->rowCount()-1;i>=0;i--)  //展示的tablewidget倒序清空
        {
            _cartTable->removeRow(i);
        }

        //更新购物车信息
        lb_cartNumContent->setText("0");
        lb_cartPriceContent->setText("0");

        emit signalCartCleaned();  //发送购物车清空信息
    }
}

void DialogCartView::btnCheckOutBtnClicked()
{
    if(_cartTable->rowCount()==0)
    {
        QMessageBox::critical(this,"错误","购物车为空！");
        return;
    }
    emit signalCartCheckOut();  //发送购物车结算消息
    this->close();
}

void DialogCartView::slotCartChanged(QTableWidgetItem *item)
{
    if(item->column()==1)  //判断修改的列为菜品数量
    {
        //如果为零则从列表中删除该行
        if(item->text()=='0')
        {
            qDebug()<<"Delete current row: "<<_cartTable->item(item->row(),0)->text();
            _cartList.removeAt(item->row());  //删除_cartList中的菜品信息
            _cartTable->removeRow(item->row());  //删除_cartTable中的菜品信息
            //qDebug()<<_cartList;

            //_cartPrice和_cartNum清零
            _cartNum = 0;
            _cartPrice = 0;

            //计算当前购物车总价
            for(int i=0;i<_cartList.size();i++)
            {
                _cartPrice+=_cartList.at(i).getSum();
            }
            //计算当前购物车总数
            for(int i=0;i<_cartList.size();i++)
            {
                _cartNum+=_cartList.at(i).getNum();
            }

            //更新购物车信息
            lb_cartNumContent->setText(QString::number(_cartNum));
            lb_cartPriceContent->setText(QString::number(_cartPrice));

            emit signalCartChanged(_cartList);  //发送购物车改变消息

            return;  //退出
        }

        qDebug()<<"row:"<<item->row()<<" "<<"column:"<<item->column();

        QTableWidgetItem *currentItemPrice = new QTableWidgetItem;
        const_cast<CartItem&>(_cartList.at(item->row())).setItemNums(item->text().toInt());
        currentItemPrice->setText(QString::number(_cartList.at(item->row()).getSum())); //重新计算当前菜品总价
        _cartTable->setItem(item->row(),2,currentItemPrice); //更新_cartTable当前菜品总价

        //_cartPrice和_cartNum清零
        _cartPrice = 0;
        _cartNum = 0;

        //计算当前购物车总价
        for(int i=0;i<_cartList.size();i++)
        {
            _cartPrice+=_cartList.at(i).getSum();
        }
        //计算当前购物车总数
        for(int i=0;i<_cartList.size();i++)
        {
            _cartNum+=_cartList.at(i).getNum();
        }

        //更新购物车信息
        lb_cartNumContent->setText(QString::number(_cartNum));
        lb_cartPriceContent->setText(QString::number(_cartPrice));

        emit signalCartChanged(_cartList);  //发送购物车改变消息
    }
}
