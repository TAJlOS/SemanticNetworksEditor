#include "gui.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <math.h>



ObjectWidget::ObjectWidget(QWidget* parent)
{
    this->setParent(parent);
    textEdit = new QLineEdit(parent);
    textEdit->setFrame(false);
    textEdit->setAlignment(Qt::AlignCenter);
    textEdit->setFont(QFont("Helvetica [Cronyx]", 8));
    connect(textEdit,SIGNAL(editingFinished()),
            this,SLOT(textEditEditingFinished()));
    textEdit->show();
    setMouseTracking(true);
    QFont font = textEdit->font();
    font.setPixelSize(fontSize);
    for(int i = 0; i < 8;i++)
        connectionRegionsFlags[i] = Q_NULLPTR;
}

ObjectWidget::~ObjectWidget()
{
    delete textEdit;
    delete object;
}

void ObjectWidget::setObject(Object *object)
{
    this->object = object;
    textEdit->setText(object->name);
}

void ObjectWidget::save(QDataStream &stream)
{
    stream << this->geometry();
    stream << object->name;
}

void ObjectWidget::save(QDataStream &stream,std::map<LinkWidget*,int> mapLinks)
{
    std::vector<int> links;
    for(auto e: connectionRegionsFlags){
        if(e == Q_NULLPTR) stream << -1;
        else{
            for(int i = 0; i < object->links.size(); i ++)
                if(object->links[i] == e->getLink()){
                    links.push_back(mapLinks[e]);
                    break;
                }
            stream << mapLinks[e];
        }
    }
    stream << int(links.size());
    for(auto e: links)
        stream << e;
}

void ObjectWidget::load(QDataStream &stream)
{
    QRect rect;
    stream >> rect;
    fixGeometry(rect);
    Object* object = new Object();
    stream >> object->name;
    setObject(object);

}

void ObjectWidget::load(QDataStream &stream,std::map<int,LinkWidget*> mapLinks)
{
    for(int i = 0; i < 8;i++){
        int j;
        stream >> j;
        if(j == -1) connectionRegionsFlags[i] = Q_NULLPTR;
        else{
            connectionRegionsFlags[i] = mapLinks[j];
        }
    }
    int linksSize;
    stream >> linksSize;
    for(int i = 0; i < linksSize;i++){
        int j;
        stream >> j;
        object->links.push_back(mapLinks[j]->getLink());
    }
}

void ObjectWidget::fixGeometry(QRect rect)
{

    this->setGeometry(rect.left(),
                      rect.top(),
                      rect.width(),
                      rect.height());
    int w = rect.width() - 2*margin;
    int h = rect.height()- 2*margin;
    bounds = QRect(margin,margin,w,h);
    boundingCircles[0] = QRect(0,0,margin*2,margin*2);
    boundingCircles[1] = QRect(w/2,0,margin*2,margin*2);
    boundingCircles[2] = QRect(w,0,margin*2,margin*2);
    boundingCircles[3] = QRect(w,h/2,margin*2,margin*2);
    boundingCircles[4] = QRect(w,h,margin*2,margin*2);
    boundingCircles[5] = QRect(w/2,h,margin*2,margin*2);
    boundingCircles[6] = QRect(0,h,margin*2,margin*2);
    boundingCircles[7] = QRect(0,h/2,margin*2,margin*2);
    double angle = M_PI_4;
    connectionRegions[0] = QRect(static_cast<int>(w*cos(-3*angle)/2+w/2+margin),
                                 static_cast<int>(h*sin(-3*angle)/2+h/2+margin)-1,
                                5,5);
    connectionRegions[1] = QRect(w/2,1,5,5);
    connectionRegions[2] = QRect(static_cast<int>(w*cos(-angle)/2+w/2+margin),
                                 static_cast<int>(h*sin(-angle)/2+h/2+margin)-1,
                                5,5);
    connectionRegions[3] = QRect(w+1,h/2,5,5);
    connectionRegions[4] = QRect(static_cast<int>(w*cos(angle)/2+w/2+margin),
                                 static_cast<int>(h*sin(angle)/2+h/2+margin)-1,
                                5,5);
    connectionRegions[5] = QRect(w/2,h+1,5,5);
    connectionRegions[6] = QRect(static_cast<int>(w*cos(3*angle)/2+w/2+margin),
                                 static_cast<int>(h*sin(3*angle)/2+h/2+margin)-1,
                                5,5);
    connectionRegions[7] = QRect(1,h/2,5,5);

    int htext = fontSize;
    int wtext = static_cast<int>(w*0.8);
    textEdit->setGeometry(w/2 +margin - wtext/2+mapToParent(QPoint(0,0)).x(),
                          h/2 +margin - htext/2+mapToParent(QPoint(0,0)).y(),
                          wtext,
                          htext);
}

void ObjectWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    if(condition == usual){
        painter.setPen(Qt::black);
        painter.drawEllipse(bounds);
        if(drawConnectingRegion){
            QPen pen2(Qt::blue);
            pen2.setWidth(2);
            painter.setPen(pen2);
            for(auto e: connectionRegions){
                painter.drawLine(e.topLeft(),e.bottomRight());
                painter.drawLine(e.topRight(),e.bottomLeft());
            }
        }
    }
    if(condition != usual &&
            !(condition == movement && dirmoveFlag == any)){

        painter.setPen(Qt::black);
        painter.drawEllipse(bounds);

        QColor color(80,160,230);
        QPen pen1(color);
        pen1.setDashPattern(QVector<qreal>(2,3));
        painter.setPen(pen1);
        painter.drawRect(bounds);
        painter.setBrush(color);
        for(auto e: boundingCircles)
            painter.drawEllipse(e);


    }
    if(condition == movement && dirmoveFlag == any){
        QPen pen(Qt::black);
        pen.setDashPattern(QVector<qreal>(2,3));
        painter.setPen(pen);
        painter.drawRect(bounds);
    }
}

void ObjectWidget::mousePressEvent(QMouseEvent *event)
{
    pressPoint = event->pos();
    if(isPointInEllipse(pressPoint)){
        if(condition == usual){
            condition = selection;
            emit changingWidget(true);
        }
        else if(condition == selection)
            condition = changing;
    }
    else{
        if(condition == selection){
            if(isPointInBoundingCircle(event->pos()) == -1){
                condition = usual;
                emit changingWidget(false);
            }
            else condition = changing;
        }
    }
    repaint();
}

void ObjectWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint point = event->pos();
    if(condition == changing)
        condition = selection;
    if(condition == movement){
        moveGeometry(point);
        condition = selection;
        repaint();
    }
}

void ObjectWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint point = event->pos();
    if(condition == usual){
        if(isPointInEllipse(point)){
            this->setCursor(Qt::SizeAllCursor);
        }
        else{
            this->setCursor(Qt::ArrowCursor);
        }
        if(bounds.contains(point)){
            drawConnectingRegion = true;
            repaint();
        }
        else{
            drawConnectingRegion = false;
            repaint();
        }
    }
    else{
        int j = -1;
        j = isPointInBoundingCircle(point);
        if(j == -1){
            if(isPointInEllipse(point)){
                this->setCursor(Qt::SizeAllCursor);
                if(condition == selection ||
                        condition == changing)
                    dirmoveFlag = any;
            }
            else{
                this->setCursor(Qt::ArrowCursor);
            }
        }
        else{
            if(condition == selection ||
                    condition == changing){
                if(j == 0 || j == 4){
                    this->setCursor(Qt::SizeFDiagCursor);
                    if(j == 0) dirmoveFlag = FDiag0;
                    if(j == 4) dirmoveFlag = FDiag4;
                }
                if(j == 2 || j == 6){
                    this->setCursor(Qt::SizeBDiagCursor);
                    if(j == 2) dirmoveFlag = BDiag2;
                    if(j == 6) dirmoveFlag = BDiag6;
                }
                if(j == 1 || j == 5){
                    this->setCursor(Qt::SizeVerCursor);
                    if(j == 1) dirmoveFlag = Ver1;
                    if(j == 5) dirmoveFlag = Ver5;
                }
                if(j == 3 || j == 7){
                    this->setCursor(Qt::SizeHorCursor);
                    if(j == 3) dirmoveFlag = Hor3;
                    if(j == 7) dirmoveFlag = Hor7;
                }
            }

        }
        if(condition == changing && point != pressPoint){
            condition = movement;
        }
        if(condition == movement){
            QRect rect = moveGeometry(point);
            fixGeometry(rect);
            repaint();
        }
    }
    if (dirmoveFlag != FDiag0 &&
            dirmoveFlag != BDiag6 &&
            dirmoveFlag != BDiag2 &&
            dirmoveFlag != Hor7 &&
            dirmoveFlag != Ver1) curPoint = point;
}

void ObjectWidget::leaveEvent(QEvent *)
{
    drawConnectingRegion = false;
    repaint();
}

void ObjectWidget::keyPressEvent(QKeyEvent *event)
{
    qDebug() << event->key();
    if(condition == selection){
        if(event->key() == Qt::Key_Delete){
            for(auto e: connectionRegionsFlags)
                e->deleteLink(this);
        }
        emit deleteMe();
    }
}

void ObjectWidget::deleteLink(LinkWidget* linkWidget)
{
    for(int i = 0; i < 8; i++)
        if(connectionRegionsFlags[i] == linkWidget){
            connectionRegionsFlags[i] = Q_NULLPTR;
            object->deleteLink(linkWidget->getLink());
        }
}

bool ObjectWidget::isPointInEllipse(QPoint point)
{
    int w = bounds.width();
    int h = bounds.height();
    // если точка лежит внутри эллипса
    double func = pow(point.x()-margin-w/2,2)/pow(w/2,2) +
         pow(point.y()-margin-h/2,2)/pow(h/2,2);
    if(func <= 1.0)
        return true;
    return false;
}

int ObjectWidget::isPointInBoundingCircle(QPoint point)
{
    int j = -1;
    for(int i = 0; i < 8; i++){
        QRect rect = boundingCircles[i];
        rect.setLeft(rect.left() - margin);
        rect.setTop(rect.top() - margin);
        rect.setRight(rect.right() + margin);
        rect.setBottom(rect.bottom() + margin);
        if(rect.contains(point)) return i;
    }
    return j;
}

int ObjectWidget::isPointInConnectingRegion(QPoint point)
{
    int j = -1;
    for(int i = 0; i < 8; i++){
        QRect rect = connectionRegions[i];
        rect.setLeft(rect.left() - margin);
        rect.setTop(rect.top() - margin);
        rect.setRight(rect.right() + margin);
        rect.setBottom(rect.bottom() + margin);
        if(rect.contains(point)) return i;
    }
    return j;
}

QRect ObjectWidget::moveGeometry(QPoint point)
{
    QRect rect = this->geometry();
    if(dirmoveFlag == any){
        rect.setTopLeft(rect.topLeft()+point-pressPoint);
        rect.setBottomRight(rect.bottomRight()+point-pressPoint);
    }
    if(dirmoveFlag == Hor3)
        rect.setRight(rect.right()+(point-curPoint).x());
    if(dirmoveFlag == Hor7)
        rect.setLeft(rect.left()+(point-curPoint).x());
    if(dirmoveFlag == Ver1)
        rect.setTop(rect.top()+(point-curPoint).y());
    if(dirmoveFlag == Ver5)
        rect.setBottom(rect.bottom()+(point-curPoint).y());

    if(dirmoveFlag == FDiag0)
        rect.setTopLeft(rect.topLeft()+point-curPoint);
    if(dirmoveFlag == FDiag4)
        rect.setBottomRight(rect.bottomRight()+point-curPoint);
    if(dirmoveFlag == BDiag2){
        rect.setTopRight(rect.topRight()+point-curPoint);
        curPoint.setX( point.x() );
    }
    if(dirmoveFlag == BDiag6){
        rect.setBottomLeft(rect.bottomLeft()+point-curPoint);
        curPoint.setY( point.y() );
    }
    return rect;
}

void ObjectWidget::textEditEditingFinished()
{
    object->name = textEdit->text();
    textEdit->clearFocus();
}

void ObjectWidget::movingLink(QPoint point)
{
    QRect rect = toGlobalRect(bounds);
    bool oldFlag = drawConnectingRegion;
    if(rect.contains(point))
        drawConnectingRegion = true;
    else drawConnectingRegion = false;
    if(oldFlag != drawConnectingRegion)
       repaint();
}

void ObjectWidget::connectingLink(QPoint point)
{
    repaint();
    LinkWidget* link = qobject_cast<LinkWidget*>(sender());
    for(int i = 0; i < 8; i++){
        if(connectionRegionsFlags[i] == Q_NULLPTR){
            QRect rect = toGlobalRect(connectionRegions[i]);
            rect.setTopLeft(rect.topLeft()-QPoint(margin,margin));
            rect.setBottomRight(rect.bottomRight()+QPoint(margin,margin));
            if(rect.contains(point)){

                link->makeConnection(rect.center(), this);
                connectionRegionsFlags[i] = link;
                return;
            }
            //qDebug()<<"Center: "<<i<<rect.center()<<" Point: "<<point;
        }
        else if(link == connectionRegionsFlags[i]){
            QRect rect = toGlobalRect(connectionRegions[i]);
            rect.setTopLeft(rect.topLeft()-QPoint(margin,margin));
            rect.setBottomRight(rect.bottomRight()+QPoint(margin,margin));
            if(!rect.contains(point)){
                if(link->breakConnection(this))
                    connectionRegionsFlags[i] = Q_NULLPTR;
                return;
            }
        }
    }
}

QRect ObjectWidget::toGlobalRect(QRect rect)
{
    rect.setTopLeft(mapToParent(rect.topLeft()));
    rect.setBottomRight(mapToParent(rect.bottomRight()));
    return rect;
}

LinkWidget::LinkWidget(QWidget* parent)
{
    this->setParent(parent);
    begin = QPoint(0,0);
    end = QPoint(0,0);
    setMouseTracking(true);

    textLinkName = new QLineEdit(parent);
    textLinkName->setFrame(false);
    textLinkName->setAlignment(Qt::AlignCenter);
    textLinkName->setFont(QFont("Helvetica [Cronyx]", 8));
    textLinkName->show();

    connect(textLinkName,SIGNAL(editingFinished()),this,SLOT(changeLinkName()));

    textLinkType = new QLineEdit(parent);
    textLinkType->setFrame(false);
    textLinkType->setAlignment(Qt::AlignCenter);
    textLinkType->setFont(QFont("Helvetica [Cronyx]", 8,QFont::Bold));
    textLinkType->show();
}

void LinkWidget::changeLinkName()
{
    link->name = textLinkName->text();
    textLinkName->clearFocus();
}

LinkWidget::~LinkWidget()
{
    delete textLinkType;
    delete textLinkName;
    delete link;
}

void LinkWidget::setLink(Link* link)
{
    this->link = link;
    QString type = toString(link->linkType);
    textLinkType->setText("Тип: "+type);
    textLinkType->setReadOnly(true);
    textLinkName->setText(link->name);
}

void LinkWidget::save(QDataStream &stream)
{
    stream << this->geometry();
    stream << link->linkType;
    stream << link->name;

    stream << begin;
    stream << end;
}

void LinkWidget::save(QDataStream &stream,std::map<ObjectWidget*,int> mapObjects)
{
    if(receiver != Q_NULLPTR) stream << mapObjects[receiver];
    else stream << -1;
    if(source != Q_NULLPTR) stream << mapObjects[source];
    else stream << -1;
}

void LinkWidget::load(QDataStream &stream)
{
    QRect rect;
    stream >> rect;
    fixGeometry(rect);
    Link* link = new Link();
    int linkType;
    stream >> linkType;
    link->linkType = LinkTypes(linkType);
    stream >> link->name;
    setLink(link);
    stream >> begin;
    stream >> end;
}

void LinkWidget::load(QDataStream &stream,std::map<int,ObjectWidget*> mapObjects)
{
    int j;
    stream >> j;
    if(j != -1){
        receiver = mapObjects[j];
        link->receiver = mapObjects[j]->getObject();
    }
    else{
        receiver = Q_NULLPTR;
        link->receiver = Q_NULLPTR;
    }
    stream >> j;
    if(j != -1){
        source = mapObjects[j];
        link->source = mapObjects[j]->getObject();
    }
    else{
        source = Q_NULLPTR;
        link->source = Q_NULLPTR;
    }
}

void LinkWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if(condition == usual){
        painter.setPen(Qt::black);
        painter.drawLine(begin, end);
        paintArrow(painter);
    }
    if(condition != usual &&
            !(condition == movement && dirmoveFlag == any)){

        painter.setPen(Qt::black);
        painter.drawLine(begin, end);
        paintArrow(painter);

        QColor color(80,160,230);
        QPen pen(color);
        pen.setDashPattern(QVector<qreal>(2,3));
        painter.setPen(pen);
        painter.drawLine(begin, end);
        painter.setBrush(color);
        for(auto e: boundingCircles)
            painter.drawEllipse(e);
    }
    if(condition == movement && dirmoveFlag == any){
        QPen pen(Qt::black);
        pen.setDashPattern(QVector<qreal>(2,3));
        painter.setPen(pen);
        painter.drawRect(bounds);
    }
}

void LinkWidget::paintArrow(QPainter &painter)
{
    // координаты вектора
    int x = (begin-end).x();
    int y = (begin-end).y();
    double len = sqrt(x*x+y*y);
    // координаты стрелок
    double angle = 20*M_PI/180;
    int x1 = static_cast<int> (x*cos(angle)-y*sin(angle));
    int y1 = static_cast<int> (x*sin(angle)+y*cos(angle));
    int x2 = static_cast<int> (x*cos(-angle)-y*sin(-angle));
    int y2 = static_cast<int> (x*sin(-angle)+y*cos(-angle));
    // нормирует длину стрелок
    x1 = static_cast<int>(x1*10/len);
    y1 = static_cast<int>(y1*10/len);
    x2 = static_cast<int>(x2*10/len);
    y2 = static_cast<int>(y2*10/len);
    QLine arrow1(QPoint(x1,y1)+end,end);
    QLine arrow2(QPoint(x2,y2)+end,end);
    painter.drawLine(arrow1);
    painter.drawLine(arrow2);
}

void LinkWidget::mousePressEvent(QMouseEvent *event)
{
    pressPoint = event->pos();
    if(isPointInLink(pressPoint)){
        if(condition == usual){
            condition = selection;
            emit changingWidget(true);
        }
        else if(condition == selection)
            condition = changing;
    }
    else{
        if(condition == selection){
            if(isPointInBoundingCircle(event->pos()) == -1){
                condition = usual;
                emit changingWidget(false);
            }
            else condition = changing;
        }
    }
    repaint();
}

void LinkWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint point = event->pos();
    if(condition == changing)
        condition = selection;
    if(condition == movement){
        moveGeometry(point);
        if(dirmoveFlag == BDiag2)
            emit connecting(mapToParent (end));
        if(dirmoveFlag == BDiag6)
            emit connecting(mapToParent (begin));
        condition = selection;
        repaint();
    }
}

void LinkWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint point = event->pos();
    if(condition == usual){
        if(isPointInLink(point)){
            this->setCursor(Qt::SizeAllCursor);
        }
        else{
            this->setCursor(Qt::ArrowCursor);
        }
    }
    else{
        int j = -1;
        j = isPointInBoundingCircle(point);
        if(j == -1){
            if(isPointInLink(point)){
                this->setCursor(Qt::SizeAllCursor);
            }
            else{
                this->setCursor(Qt::ArrowCursor);
            }
            if(condition == selection ||
                    condition == changing)
                dirmoveFlag = any;
        }
        else{
            if(condition == selection ||
                    condition == changing){
                if(j == 0 || j == 1){
                    this->setCursor(Qt::PointingHandCursor);
                    // 0 - begin, 1 - end
                    if(j == 0) dirmoveFlag = BDiag6;
                    if(j == 1) dirmoveFlag = BDiag2;
                }
            }
        }
        if(condition == changing && point != pressPoint){
            condition = movement;
        }
        if(condition == movement){

            moveGeometry(point);
            repaint();
        }
    }
    if (dirmoveFlag != BDiag6 && dirmoveFlag != BDiag2)
        curPoint = point;
    if(dirmoveFlag == BDiag2)
        emit movingWidget(mapToParent(end));
    if(dirmoveFlag == BDiag6)
        emit movingWidget(mapToParent(begin));
}

void LinkWidget::keyPressEvent(QKeyEvent *event)
{
    if(condition == selection){
        if(event->key() == Qt::Key_Delete){
            if(receiver != Q_NULLPTR){
                receiver->deleteLink(this);
            }
            if(source != Q_NULLPTR){
                source->deleteLink(this);
            }
        }
        emit deleteMe();
    }
}


bool LinkWidget::isPointInLink(QPoint point)
{
    // уравнение прямой ax+bx+c=0
    int a, b, c;
    a = end.y() - begin.y();
    b = -(end.x() - begin.x());
    c = -a*begin.x() - b*begin.y();
    int x = point.x();
    int y = 0;
    // находим точку на прямой с координатой
    // x или y как у point
    if(b != 0)
        y = static_cast<int>(-(a*x + c)/b);
    else{
        x = end.x();
        y = point.y();
    }
    QRect rect(x-margin,y-margin,margin*2,margin*2);
    if(rect.contains(point))
        return true;
    return false;
}

int LinkWidget::isPointInBoundingCircle(QPoint point)
{
    int j = -1;
    for(int i = 0; i < 2; i++){
        QRect rect = boundingCircles[i];
        rect.setLeft(rect.left() - margin);
        rect.setTop(rect.top() - margin);
        rect.setRight(rect.right() + margin);
        rect.setBottom(rect.bottom() + margin);
        if(rect.contains(point)) return i;
    }
    return j;
}

void LinkWidget::fixGeometry(QRect rect)
{
    this->setGeometry(rect.left(),
                      rect.top(),
                      rect.width(),
                      rect.height());
    int w = rect.width()- 4*margin;
    int h = rect.height()- 4*margin;
    bounds = QRect(2*margin,2*margin,w,h);
    boundingCircles[0] = QRect(begin.x() - margin,begin.y() - margin,
                               margin*2,margin*2);
    boundingCircles[1] = QRect(end.x() - margin,end.y() - margin,
                               margin*2,margin*2);
    h = fontSize;
    w = 50;
    textLinkType->setGeometry(rect.width()/2 - w/2+mapToParent(QPoint(0,0)).x(),
                          rect.height()/2 - h/2+mapToParent(QPoint(0,0)).y()-h/2,
                          w,
                          h);
    w = 120;
    textLinkName->setGeometry(rect.width()/2 - w/2+mapToParent(QPoint(0,0)).x(),
                              rect.height()/2 - h/2+mapToParent(QPoint(0,0)).y()+h/2,
                              w,
                              h);

}

void LinkWidget::moveGeometry(QPoint point)
{
    QRect geometry = this->geometry();
    if(dirmoveFlag == any){
        int x = (point - pressPoint).x();
        int y = (point - pressPoint).y();
        geometry.setX(geometry.x() + x);
        geometry.setY(geometry.y() + y);
        geometry.setRight(geometry.right() + x);
        geometry.setBottom(geometry.bottom() + y);
    }
    if(dirmoveFlag == BDiag2){
        //int dx = (point-curPoint).x()-(end-curPoint).x();
        //int dy = (point-curPoint).y()-(end-curPoint).y();
        int dx = (point-end).x();
        int dy = (point-end).y();
        if((end.x()+dx > begin.x()) && (end.y()+dy < begin.y())){
            geometry.setRight(geometry.right()+dx);
            geometry.setTop(geometry.top()+dy);
            end.setX(end.x() + dx);
            begin.setY(begin.y() - dy);
            //curPoint.setX( point.x() );
        }
        if((end.x()+dx <= begin.x()) && (end.y()+dy < begin.y())){
            geometry.setLeft(geometry.left()+dx);
            geometry.setTop(geometry.top()+dy);
            begin.setY(begin.y() - dy);
            begin.setX(begin.x() - dx);
        }
        if((end.x()+dx > begin.x()) && (end.y()+dy >= begin.y())){
            geometry.setRight(geometry.right()+dx);
            geometry.setBottom(geometry.bottom()+dy);
            end.setX(end.x() + dx);
            end.setY(end.y() + dy);
            //curPoint = point;
        }
        if((end.x()+dx <= begin.x()) && (end.y()+dy >= begin.y())){
            geometry.setLeft(geometry.left()+dx);
            geometry.setBottom(geometry.bottom()+dy);
            begin.setX(begin.x() - dx);
            end.setY(end.y() + dy);
            //curPoint.setY( point.y() );
        }
    }
    if(dirmoveFlag == BDiag6){
        //int dx = (point-curPoint).x()-(begin-curPoint).x();
        //int dy = (point-curPoint).y()-(begin-curPoint).y();
        int dx = (point-begin).x();
        int dy = (point-begin).y();
        if((begin.x()+dx < end.x()) && (begin.y()+dy > end.y())){
            geometry.setLeft(geometry.left()+dx);
            geometry.setBottom(geometry.bottom()+dy);
            end.setX(end.x() - dx);
            begin.setY(begin.y() + dy);
            //curPoint.setY( point.y() );
            //curPoint.setY(begin.y());
        }
        if((begin.x()+dx >= end.x()) && (begin.y()+dy > end.y())){
            geometry.setRight(geometry.right()+dx);
            geometry.setBottom(geometry.bottom()+dy);
            begin.setX(begin.x() + dx);
            begin.setY(begin.y() + dy);
            //curPoint = point;
            //curPoint = begin;
        }
        if((begin.x()+dx < end.x()) && (begin.y()+dy <= end.y())){
            geometry.setLeft(geometry.left()+dx);
            geometry.setTop(geometry.top()+dy);
            end.setY(end.y() - dy);
            end.setX(end.x() - dx);
        }
        if((begin.x()+dx >= end.x()) && (begin.y()+dy <= end.y())){
            geometry.setRight(geometry.right()+dx);
            geometry.setTop(geometry.top()+dy);
            end.setY(end.y() - dy);
            begin.setX(begin.x() + dx);
            //curPoint.setX( point.x() );
            //curPoint.setX(begin.x());
        }
        //curPoint = begin;
    }
    if(dirmoveFlag != none)
        fixGeometry(geometry);
}

void LinkWidget::makeConnection(QPoint point, ObjectWidget* objectWidget)
{
    qDebug()<<point<<mapToParent(end)<<end<<(dirmoveFlag == BDiag2);
    if(dirmoveFlag == BDiag2){
        curPoint = end;
        moveGeometry((point - mapToParent(end))+end);
    }
    if(dirmoveFlag == BDiag6){
        curPoint = end;
        moveGeometry((point - mapToParent(begin))+begin);
    }

    if(dirmoveFlag == BDiag2){
        receiver = objectWidget;
        link->receiver = objectWidget->getObject();
        qDebug()<<"receiver: "<<link->receiver->name;
    }
    else{
        source = objectWidget;
        link->source = objectWidget->getObject();
        qDebug()<<"source: "<<link->source->name;
    }
    if(receiver != Q_NULLPTR && source != Q_NULLPTR){
        link->source->addLink(link);
        link->receiver->addLink(link);
        qDebug()<<"source: "<<link->source->name<<" receiver: "<<link->receiver->name;
    }
    repaint();
}

bool LinkWidget::breakConnection(ObjectWidget* objectWidget)
{
    if(dirmoveFlag == BDiag2 && receiver == objectWidget){
        qDebug()<<"Разрыв соединения receiver: "<<link->receiver->name;
        receiver = Q_NULLPTR;
        link->receiver = Q_NULLPTR;
        return true;
    }
    if(dirmoveFlag == BDiag6 && source == objectWidget){
        qDebug()<<"Разрыв соединения source: "<<link->source->name;
        source = Q_NULLPTR;
        link->source = Q_NULLPTR;
        return true;
    }
    return false;
}

void LinkWidget::deleteLink(ObjectWidget* objectWidget)
{
    if(receiver == objectWidget){
        receiver = Q_NULLPTR;
        link->receiver = Q_NULLPTR;
    }
    if(source == objectWidget){
        source = Q_NULLPTR;
        link->source = Q_NULLPTR;
    }
}
