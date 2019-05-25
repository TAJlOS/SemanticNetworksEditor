#ifndef GUI_H
#define GUI_H

#include <QWidget>
#include <QLineEdit>
#include <QLayout>
#include "core.h"
#include <QTextStream>
#include <QFile>

enum DirMoveFlags{
    none,
    FDiag0,
    FDiag4,
    BDiag2,
    BDiag6,
    Ver1,
    Ver5,
    Hor3,
    Hor7,
    any
};

enum Condition
{
    usual,
    selection,
    changing,
    movement
};

class LinkWidget;

class ObjectWidget :public QWidget
{
    Q_OBJECT
public:
    explicit ObjectWidget(QWidget* parent = 0);
    ~ObjectWidget();
    void setObject(Object* object);
    void setCondition(Condition condition){this->condition = condition;}
    Object* getObject(){return object;}
    void fixGeometry(QRect rect);
    static const int margin = 4;
    void save(QDataStream &stream);
    void save(QDataStream &stream,std::map<LinkWidget*,int> mapLinks);
    void load(QDataStream &stream);
    void load(QDataStream &stream,std::map<int,LinkWidget*> mapLinks);
    void deleteLink(LinkWidget* linkWidget);
signals:
    void changingWidget(bool);
    void deleteMe();
protected slots:
    void textEditEditingFinished();
    void connectingLink(QPoint point);
    void movingLink(QPoint point);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);
    void leaveEvent(QEvent *);
    void keyPressEvent(QKeyEvent *);
private:

    bool isPointInEllipse(QPoint point);
    int isPointInBoundingCircle(QPoint point);
    int isPointInConnectingRegion(QPoint point);
    QRect moveGeometry(QPoint point);
    QRect toGlobalRect(QRect rect);
    QLineEdit* textEdit = Q_NULLPTR;
    Object* object = Q_NULLPTR;
    bool editingFlag = false;
    bool movingFlag = false;
    bool showEditing = false;
    QPoint pressPoint, curPoint;

    QRect boundingCircles[8];
    QRect connectionRegions[8];
    LinkWidget* connectionRegionsFlags[8];
    QRect bounds;
    DirMoveFlags dirmoveFlag = none;
    Condition condition = usual;
    int fontSize = 14;
    bool drawConnectingRegion = false;
};

class LinkWidget :public QWidget
{
    Q_OBJECT
public:
    explicit LinkWidget(QWidget* parent = 0);
    ~LinkWidget();
    void setLink(Link* link);
    Link* getLink(){return link;}
    const static int margin = 4;
    void fixGeometry(QRect rect);
    void setCondition(Condition condition){this->condition = condition;}
    void makeConnection(QPoint, ObjectWidget*);
    bool breakConnection(ObjectWidget*);
    void save(QDataStream &stream);
    void save(QDataStream &stream,std::map<ObjectWidget*,int> mapObjects);
    void load(QDataStream &stream);
    void load(QDataStream &stream,std::map<int,ObjectWidget*> mapObjects);
    void deleteLink(ObjectWidget* objectWidget);
    QPoint begin, end;
signals:
    void changingWidget(bool);
    void movingWidget(QPoint point);
    void connecting(QPoint point);
    void deleteMe();
protected slots:
    void changeLinkName();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent *event);
private:
    bool isPointInLink(QPoint point);
    int isPointInBoundingCircle(QPoint point);
    void paintArrow(QPainter &painter);
    void moveGeometry(QPoint point);
    QLineEdit* textLinkType = Q_NULLPTR;
    QLineEdit* textLinkName = Q_NULLPTR;
    Link* link = Q_NULLPTR;
    QPoint pressPoint, curPoint;

    // boundingCircles[0] == begin
    // boundingCircles[1] == end
    QRect boundingCircles[2];
    QRect bounds;
    DirMoveFlags dirmoveFlag = none;
    Condition condition = usual;
    int fontSize = 14;
    ObjectWidget* source = Q_NULLPTR;
    ObjectWidget* receiver = Q_NULLPTR;
};


#endif // GUI_H
