/////////////////////////////////////////////////////////////////////////////
// Authored by Jeong-Mo Hong for CSE4060 course at Dongguk University CSE  //
// jeongmo.hong@gmail.com                                                  //
// Do whatever you want license.                                           //
/////////////////////////////////////////////////////////////////////////////

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

#include <Qt3DRender/QMesh>
#include <Qt3DRender/qabstractattribute.h>
#include <iostream>
#include <QtCore/QDebug>

#include "Vector2D.h"

SceneModifier::SceneModifier(Qt3DCore::QEntity *rootEntity)
	: m_rootEntity(rootEntity), idle_timer_(new QTimer(this)), nn_(2 + 2 + 0, 4, 1), target_x_(1), target_y_(0), target_z_(1)
{
	// initialize scene
    Qt3DRender::QPhongMaterial *red_material = new Qt3DRender::QPhongMaterial();
    red_material->setDiffuse(QColor(QRgb(0xCC0000)));

    Qt3DRender::QPhongMaterial *blue_material = new Qt3DRender::QPhongMaterial();
    blue_material->setDiffuse(QColor(QRgb(0x0000CC)));

	Qt3DRender::QPhongMaterial *ground_material = new Qt3DRender::QPhongMaterial();
	ground_material->setDiffuse(QColor(QRgb(0xFFF44F)));

	ground_entity_ = new Qt3DCore::QEntity(m_rootEntity);
    target_entity_ = new Qt3DCore::QEntity(m_rootEntity);
    agent_entity_ = new Qt3DCore::QEntity(m_rootEntity);

	ground_transform_ = new Qt3DCore::QTransform();
	ground_transform_->setTranslation(QVector3D(0.5f, -0.25, 0.5f));
	ground_transform_->setScale3D(QVector3D(1.3f, 0.1f, 1.3f));

    target_transform_ = new Qt3DCore::QTransform();
    target_transform_->setScale(0.013f);
	target_transform_->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), -90.0f));
    target_transform_->setTranslation(QVector3D(target_x_, target_y_, target_z_));

    agent_transform_ = new Qt3DCore::QTransform();
    agent_transform_->setScale(0.005f);
	agent_transform_->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), -90.0f));
    agent_transform_->setTranslation(QVector3D(ag_.pos_x_, ag_.pos_y_, ag_.pos_z_));

	Qt3DRender::QMesh *akiko_mesh = new Qt3DRender::QMesh();
	akiko_mesh->setSource(QUrl::fromLocalFile("ori2C_whole_Simp.obj"));

    target_entity_->addComponent(target_transform_);
    target_entity_->addComponent(red_material);
    target_entity_->addComponent(akiko_mesh);

	Qt3DRender::QMesh *valut_boy_mesh = new Qt3DRender::QMesh();
	valut_boy_mesh->setSource(QUrl::fromLocalFile("VaultBoy_Simp.obj"));

    agent_entity_->addComponent(agent_transform_);
    agent_entity_->addComponent(blue_material);
    agent_entity_->addComponent(valut_boy_mesh);

	ground_entity_->addComponent(ground_material);
	ground_entity_->addComponent(ground_transform_);
	ground_entity_->addComponent(new Qt3DRender::QCuboidMesh());

    distance_ = (Vector2D<double>(ag_.pos_x_, ag_.pos_z_) - Vector2D<double>(target_x_, target_x_)).getMagnitude();

    update();

    QObject::connect(idle_timer_, SIGNAL(timeout()), SLOT(update()));

    idle_timer_->setInterval(10);
    idle_timer_->start();
}

SceneModifier::~SceneModifier()
{
}

#define NUM_TRAIN 1000000 // Use 1000000

void SceneModifier::update()
{
    // move target periodically (or probabilistically)

    idle_timer_->stop();

    static bool first = true;

    int repeat = 1;

    if (first == true)
    {
        repeat = (double)NUM_TRAIN / (double)15;
    }
    
    for (int r = 0; r < repeat;r ++)
    {
        D prob_move = 0.01;

        if (first == true) prob_move = 1.0;

        if ((D)rand()/RAND_MAX < prob_move)
        {
            moveTarget();
        }

        for (int i = 0; i < 16; i++)
            updateSubstep(false);
    }

    first = false;

    idle_timer_->start();
}

void SceneModifier::updateSubstep(const bool print)
{
    VectorND<D> input;
    input.initialize(4);

    input[0] = ag_.pos_x_;
    input[1] = ag_.pos_z_;
    input[2] = target_x_;
    input[3] = target_z_;

    nn_.setInputVector(input);
    nn_.feedForward();

    VectorND<D> output;
    nn_.copyOutputVector(output, false);

    //   if(print == true) std::cout << output << std::endl;

    const int selected_dir = nn_.getIXProbabilistic(output);
    //if (print == true) std::cout << selected_dir << " " << std::flush;

    const double dt = 0.0002;

    switch (selected_dir)
    {
    case 0:
        ag_.updatePosition(1, 0, 0, dt);
        break;
    case 1:
        ag_.updatePosition(-1, 0, 0, dt);
        break;
    case 2:
        ag_.updatePosition(0, 0, 1, dt);
        break;
    case 3:
        ag_.updatePosition(0, 0, -1, dt);
        break;
    default:
        std::cout << "Wrong dimension" << std::endl;
    }

    const double new_distance_ = (Vector2D<double>(ag_.pos_x_, ag_.pos_z_) - Vector2D<double>(target_x_, target_z_)).getMagnitude();

    const double reward_value = distance_ - new_distance_;

    agent_transform_->setTranslation(QVector3D(ag_.pos_x_, ag_.pos_y_, ag_.pos_z_));

    if (new_distance_ < 0.1)
    {
        distance_ = new_distance_;

        moveTarget(); // move target when they are too close

        return; // don't reward when they are too close
    }

    VectorND<double> reward_vector(output); // reward_vector is initialized by output

    for (int d = 0; d < reward_vector.num_dimension_; d++)
    {
        if (selected_dir == d)
        {
//          reward_vector[d] = ________________ ? 0.999 : 0.001;
			reward_vector[d] = reward_value > 0 ? 0.999 : 0.001;
        }
        else
        {
            reward_vector[d] = reward_vector[d] < 0.001 ? 0.001 : reward_vector[d];
        }
    }

    const int max_tr = NUM_TRAIN;

    static int counter = 0;
    const int one_percent = (double)max_tr / 100.0;

    static int itr = 0;
    if (itr < max_tr) // train
    {
        itr++;
        counter++;

        if (counter == one_percent)
        {
            printf("%f percent \n", (double)itr / (double)max_tr * 100.0);

            counter = 0;
        }

        nn_.propBackward(reward_vector);
    }

    distance_ = new_distance_;
}

void SceneModifier::moveTarget()
{
    target_x_ = (D)rand() / RAND_MAX, target_z_ = (D)rand() / RAND_MAX;
    target_transform_->setTranslation(QVector3D(target_x_, 0.0f, target_z_));
}