#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <iostream>
#include <QLayout>
#include <QLineEdit>
#include <QSignalMapper>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <map>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setGeometry(100,100,1024,768);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    createContextMenu();
    createMainMenu();
    this->setStyleSheet("QLineEdit { border: none }");
    this->setStyleSheet("ObjectWidget {background-color:white}");
    this->setStyleSheet("MainWindow {background-color:white}");

    graph = new Graph();
    connect(this,SIGNAL(changingWidget(bool)),this,SLOT(changeEditedWidget(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete contextMenu;
    delete graph;
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
}

void MainWindow::createContextMenu()
{
    contextMenu = new QMenu(this);
    QAction* actionPaintNewObject = contextMenu->addAction(tr("Создать объект"));
    linkMenu = new QMenu(contextMenu);
    linkMenu->setTitle(tr("Создать связь"));
    connect(actionPaintNewObject,
            SIGNAL(triggered()),
            SLOT(paintNewObject()));

    contextMenu->addMenu(linkMenu);
    QSignalMapper* linkMenuSignalMapper = new QSignalMapper(this);
    connect(linkMenuSignalMapper, SIGNAL(mapped(int)),
            this, SLOT(paintNewLink(int)));
    QAction* genLink = linkMenu->addAction(tr("Gen"));
    connect(genLink,SIGNAL(triggered()),linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(genLink,Gen);

    QAction* sitLink = linkMenu->addAction(tr("Sit"));
    connect(sitLink,SIGNAL(triggered()),linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(sitLink,Sit);

    QAction* negLink = linkMenu->addAction(tr("Neg"));
    connect(negLink,SIGNAL(triggered()),linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(negLink,Neg);

    QAction* insLink = linkMenu->addAction(tr("Ins"));
    connect(insLink,SIGNAL(triggered()), linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(insLink,Ins);

    QAction* comLink = linkMenu->addAction(tr("Com"));
    connect(comLink,SIGNAL(triggered()),linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(comLink,Com);

    QAction* corLink = linkMenu->addAction(tr("Cor"));
    connect(corLink,SIGNAL(triggered()), linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(corLink,Cor);

    QAction* finLink = linkMenu->addAction(tr("Fin"));
    connect(finLink,SIGNAL(triggered()),linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(finLink,Fin);

    QAction* causLink = linkMenu->addAction(tr("Caus"));
    connect(causLink,SIGNAL(triggered()),linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(causLink,Caus);

    QAction* potLink = linkMenu->addAction(tr("Pot"));
    connect(potLink,SIGNAL(triggered()),linkMenuSignalMapper,SLOT(map()));
    linkMenuSignalMapper->setMapping(potLink,Pot);

    QAction* actionDialogWindow = contextMenu->addAction(tr("Найти путь"));
    connect(actionDialogWindow,
            SIGNAL(triggered()),
            SLOT(createDialogWindow()));
}

void MainWindow::createMainMenu()
{
    connect(ui->actionSaveAs,SIGNAL(triggered()),this,SLOT(saveFileAs()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(openFile()));
    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(exit()));
    connect(ui->actionClose,SIGNAL(triggered()),this,SLOT(closeFile()));
    connect(ui->actionSave,SIGNAL(triggered()),this,SLOT(saveFile()));
}

void MainWindow::saveFileAs()
{
    QString str = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Сохранить файл как"), "");
    saveSemanticNetwork(str);
    filename = str;
}

void MainWindow::openFile()
{
    QString str = QFileDialog::getOpenFileName(Q_NULLPTR, tr("Открыть файл"), "");
    loadSemanticNetwork(str);
    filename = str;
}

void MainWindow::closeFile()
{
    QMessageBox* message = new QMessageBox(this);
    message->setText("При закрытии документа несохранённые изменения будут утеряны.");
    message->setInformativeText("Вы действительно хотите закрыть документ?");
    message->setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    int ret = message->exec();
    delete message;
    if(ret == QMessageBox::Ok){
        clear();
        filename = QString("");
    }
}

void MainWindow::saveFile()
{
    if(filename != QString(""))
        saveSemanticNetwork(filename);
    else saveFileAs();
}

void MainWindow::exit()
{
    close();
}

void MainWindow::clear()
{
    for(auto object: objectWidgets)
        delete object;
    objectWidgets.clear();
    for(auto link: linkWidgets)
        delete link;
    linkWidgets.clear();
    repaint();
}

void MainWindow::createDialogWindow()
{
    Dialog* window = new Dialog(this);
    connect(window,SIGNAL(findPath(QString,QString,bool)),
            this,SLOT(findPath(QString,QString,bool)));
    window->show();
}

void MainWindow::findPath(QString begin,QString end,bool flag)
{
    Object* objBegin = Q_NULLPTR;
    Object* objEnd = Q_NULLPTR;
    Dialog* dialog = qobject_cast<Dialog*>(sender());
    for(auto e: objectWidgets){
        if(e->getObject()->name == begin) objBegin = e->getObject();
        if(e->getObject()->name == end) objEnd = e->getObject();
    }
    if(objBegin != Q_NULLPTR && objEnd != Q_NULLPTR){
        std::vector<std::vector<QString>> paths =
                graph->findPaths(objBegin,objEnd,flag);
        if(paths.size() == 0){
            dialog->setOutput(QString("Не существует путей между понятиями ")
                              +begin+QString(" и ")+end);
        }
        else{
            QString text;
            for(uint i = 0; i < paths.size();i++){
                text+="Путь ";
                text+=std::to_string(i+1).c_str();
                text+="\n";
                for(auto str:paths[i]){
                    text+=str+" ";
                }
                text+="\n";
            }
            dialog->setOutput(text);
        }
    }
    else{
        if(objBegin == Q_NULLPTR && objEnd == Q_NULLPTR)
            dialog->setOutput(tr("Обоих понятий не существует"));
        else{
            if(objBegin == Q_NULLPTR)
                dialog->setOutput(QString("Понятия ")+begin+
                                  QString("не существует"));
            else {
                dialog->setOutput(QString("Понятия ")+end+
                                  QString("не существует"));
            }
        }
    }
}

void MainWindow::paintNewObject()
{
    createObjectFlag = true;
}

void MainWindow::paintNewLink(int linkType)
{
    createdLinkType = LinkTypes(linkType);
    createLinkFlag = true;
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::RightButton) {
        contextMenu->exec(event->globalPos());
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        if(createObjectFlag){
            Object* newObject = new Object();
            newObject->name = "Новый объект";
            int w = 150;
            int h = 50;
            int margin = ObjectWidget::margin;
            //newObject->setGeometry(rect);
            ObjectWidget* newObjectWidget = new ObjectWidget(this);
            newObjectWidget->fixGeometry(QRect(event->pos().x() - w/2-margin,
                                               event->pos().y() - h/2-margin,
                                               w+2*margin,
                                               h+2*margin));
            newObjectWidget->setObject(newObject);
            connect(newObjectWidget,SIGNAL(changingWidget(bool)),
                    this,SLOT(changeEditedWidget(bool)));
            connect(newObjectWidget,SIGNAL(deleteMe()),
                    this,SLOT(deleteObjectWidget()));
            graph->push_back(newObject);
            objectWidgets.push_back(newObjectWidget);
            reconnect();
            newObjectWidget->show();
            createObjectFlag = false;
        }
        else if(createLinkFlag){
            Link* newLink = new Link();
            newLink->linkType = createdLinkType;
            newLink->name = "Новая связь";
            LinkWidget* newLinkWidget = new LinkWidget(this);
            int w = 70;
            int h = 70;
            int margin = LinkWidget::margin;
            newLinkWidget->begin = QPoint(margin,h+margin);
            newLinkWidget->end = QPoint(w+margin,margin);
            newLinkWidget->fixGeometry(QRect(event->pos().x() - w/2-2*margin,
                                       event->pos().y() - h/2-2*margin,
                                       w+4*margin,
                                       h+4*margin));
            newLinkWidget->setLink(newLink);
            linkWidgets.push_back(newLinkWidget);
            connect(newLinkWidget,SIGNAL(changingWidget(bool)),
                    this,SLOT(changeEditedWidget(bool)));
            connect(newLinkWidget,SIGNAL(deleteMe()),
                    this,SLOT(deleteLinkObject()));
            reconnect();
            newLinkWidget->show();
            createLinkFlag = false;
        }
        else{
            emit changingWidget(false);
        }
    }
}

void MainWindow::saveSemanticNetwork(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << int(objectWidgets.size());
    stream << int(linkWidgets.size());
    for(auto e: objectWidgets)
        e->save(stream);
    for(auto e: linkWidgets)
        e->save(stream);
    std::map<ObjectWidget*,int> mapObjects;
    for(int i = 0; i < objectWidgets.size();i++)
        mapObjects[objectWidgets[i]] = i;
    std::map<LinkWidget*,int> mapLinks;
    for(int i = 0; i < linkWidgets.size();i++)
        mapLinks[linkWidgets[i]] = i;
    for(auto e: objectWidgets)
        e->save(stream,mapLinks);
    for(auto e: linkWidgets)
        e->save(stream,mapObjects);
    file.close();
}

void MainWindow::loadSemanticNetwork(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    int n_objects, n_links;
    stream >> n_objects;
    stream >> n_links;
    std::map<int,ObjectWidget*> mapObjects;
    for(int i = 0; i < n_objects;i++){
        ObjectWidget* objectWidget = new ObjectWidget(this);
        objectWidget->load(stream);
        objectWidget->show();
        connect(objectWidget,SIGNAL(changingWidget(bool)),
                this,SLOT(changeEditedWidget(bool)));
        connect(objectWidget,SIGNAL(deleteMe()),
                this,SLOT(deleteObjectWidget()));
        objectWidgets.push_back(objectWidget);
        mapObjects[i] = objectWidget;

    }
    std::map<int,LinkWidget*> mapLinks;
    for(int i = 0; i < n_links;i++){
        LinkWidget* linkWidget = new LinkWidget(this);
        linkWidget->load(stream);
        linkWidget->show();
        connect(linkWidget,SIGNAL(changingWidget(bool)),
                this,SLOT(changeEditedWidget(bool)));
        connect(linkWidget,SIGNAL(deleteMe()),
                this,SLOT(deleteLinkObject()));
        linkWidgets.push_back(linkWidget);
        mapLinks[i] = linkWidget;
    }
    for(int i = 0; i < objectWidgets.size();i++)
        objectWidgets[i]->load(stream,mapLinks);
    for(int i = 0; i < linkWidgets.size();i++)
        linkWidgets[i]->load(stream,mapObjects);
    reconnect();
    file.close();
}

void MainWindow::deleteObjectWidget()
{
    ObjectWidget* objectWidget = qobject_cast<ObjectWidget*>(sender());
    for(int i = 0; i < objectWidgets.size();i++)
        if(objectWidgets[i] == objectWidget)
            objectWidgets.erase(objectWidgets.begin()+i);
    graph->erase(objectWidget->getObject());
    delete objectWidget;
    repaint();
}

void MainWindow::deleteLinkWidget()
{
    LinkWidget* linkWidget = qobject_cast<LinkWidget*>(sender());
    for(int i = 0; i < linkWidgets.size();i++)
        if(linkWidgets[i] == linkWidget)
            linkWidgets.erase(linkWidgets.begin()+i);
    delete linkWidget;
    repaint();
}

void MainWindow::reconnect()
{
    for(auto objectWidget: objectWidgets){
        for(auto linkWidget: linkWidgets){
            connect(linkWidget,SIGNAL(connecting(QPoint)),
                    objectWidget,SLOT(connectingLink(QPoint)));
            connect(linkWidget,SIGNAL(movingWidget(QPoint)),
                    objectWidget,SLOT(movingLink(QPoint)));
        }
    }
}

void MainWindow::changeEditedWidget(bool flag)
{
    if(flag && editedWidget == Q_NULLPTR)
        editedWidget = static_cast<QWidget*>(sender());
    else if(editedWidget != Q_NULLPTR){
        if(editedWidget->metaObject()->className() == QString("ObjectWidget"))
            static_cast<ObjectWidget*>(editedWidget)->setCondition(usual);
        if(editedWidget->metaObject()->className() == QString("LinkWidget"))
            static_cast<LinkWidget*>(editedWidget)->setCondition(usual);
        editedWidget->repaint();
        if(flag)
            editedWidget = static_cast<QWidget*>(sender());
        else editedWidget = Q_NULLPTR;
    }
}

