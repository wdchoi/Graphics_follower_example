
/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "scenemodifier.h"

#include <QGuiApplication>

#include <window.h>
#include <Qt3DCore/qcamera.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qcameralens.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QCommandLinkButton>
#include <QtGui/QScreen>

#include <Qt3DInput/QInputAspect>

#include <Qt3DRender/qtorusmesh.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qsceneloader.h>

#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DRender/qframegraph.h>
#include <Qt3DRender/qforwardrenderer.h>
#include <Qt3DRender/QPointLight>
#include <QMediaPlayer>


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Window *view = new Window();
    QWidget *container = QWidget::createWindowContainer(view);
    QSize screenSize = view->screen()->size();
    container->setMinimumSize(QSize(200, 100));
    container->setMaximumSize(screenSize);

    QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addWidget(container, 1);
    hLayout->addLayout(vLayout);

    widget->setWindowTitle(QStringLiteral("Basic shapes"));

    Qt3DCore::QAspectEngine engine;
    engine.registerAspect(new Qt3DRender::QRenderAspect());
    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    engine.registerAspect(input);
    QVariantMap data;
    data.insert(QStringLiteral("surface"), QVariant::fromValue(static_cast<QSurface *>(view)));
    data.insert(QStringLiteral("eventSource"), QVariant::fromValue(view));
    engine.setData(data);

    // Root entity
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();

    // Camera
    Qt3DCore::QCamera *cameraEntity = new Qt3DCore::QCamera(rootEntity);
    cameraEntity->setObjectName(QStringLiteral("cameraEntity"));

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 2, 0));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0.5, 0, 0.5));
    input->setCamera(cameraEntity);

    Qt3DCore::QEntity *light_entity = new Qt3DCore::QEntity(rootEntity);

    // light
 /*   Qt3DRender::QPointLight *light1 = new Qt3DRender::QPointLight();
    light1->setColor(Qt::white);
    light1->setIntensity(0.8f);

    light_entity->addComponent(light1);

    Qt3DCore::QTransform *light_transform = new Qt3DCore::QTransform();
    light_transform->setTranslation(QVector3D(0.0f, -10.0f, 10.0f));

    light_entity->addComponent(light_transform);*/

    // FrameGraph
    Qt3DRender::QFrameGraph *frameGraph = new Qt3DRender::QFrameGraph();
    Qt3DRender::QForwardRenderer *forwardRenderer = new Qt3DRender::QForwardRenderer();

    forwardRenderer->setCamera(cameraEntity);
    forwardRenderer->setClearColor(QColor(QRgb(0x77B5FE)));
    frameGraph->setActiveFrameGraph(forwardRenderer);

    // Setting the FrameGraph
    rootEntity->addComponent(frameGraph);

    // Scenemodifier
    SceneModifier *modifier = new SceneModifier(rootEntity);

    //view->cuboidTransform = modifier->cuboidTransform;

    // Set root object of the scene
    engine.setRootEntity(rootEntity);

    // Create control widgets
    QCommandLinkButton *info = new QCommandLinkButton();
    info->setText(QStringLiteral("Qt3D"));
    info->setDescription(QStringLiteral("Example"));
    info->setIconSize(QSize(0, 0));

    //QCheckBox *cuboidCB = new QCheckBox(widget);
    //cuboidCB->setChecked(true);
    //cuboidCB->setText(QStringLiteral("Cuboid"));

    vLayout->addWidget(info);
    //vLayout->addWidget(cuboidCB);    

    //QObject::connect(cuboidCB, &QCheckBox::stateChanged,
    //    modifier, &SceneModifier::enableCuboid);

 /*   cuboidCB->setChecked(true);*/
  
    // Show window
    widget->show();
    widget->resize(1200, 800);

    // Update the aspect ratio
    QSize widgetSize = container->size();
    float aspectRatio = float(widgetSize.width()) / float(widgetSize.height());
    cameraEntity->lens()->setPerspectiveProjection(45.0f, aspectRatio, 0.1f, 1000.0f);

    // background music
    //QMediaPlayer *player = new QMediaPlayer;
    //player->setMedia(QUrl::fromLocalFile("Alan Silvestri-18-The Avengers-320k.mp3"));
    //player->setVolume(50);
    //player->play();

    return app.exec();
}
