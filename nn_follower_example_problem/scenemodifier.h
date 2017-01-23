#pragma once

#include <QtCore/QObject>

#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>

#include <Qt3DRender/QTorusMesh>
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QCuboidMesh>
/////////////////////////////////////////////////////////////////////////////
// Authored by Jeong-Mo Hong for CSE4060 course at Dongguk University CSE  //
// jeongmo.hong@gmail.com                                                  //
// Do whatever you want license.                                           //
/////////////////////////////////////////////////////////////////////////////

#include <Qt3DRender/QSphereMesh>
#include <Qt3DRender/QPhongMaterial>
#include <QTimer>
#include "NeuralNetwork.h"

class Agent
{
public:
    double pos_x_, pos_y_, pos_z_;

    Agent()
        : pos_x_(0.5), pos_y_(0), pos_z_(0.5)
    {}

    void updatePosition(const double v_x, const double v_y, const double v_z, const double dt)
    {
        pos_x_ += v_x * dt;
        pos_y_ += v_y * dt;
        pos_z_ += v_z * dt;
    }
};

class SceneModifier : public QObject
{
	Q_OBJECT

public:
	explicit SceneModifier(Qt3DCore::QEntity *rootEntity);
	~SceneModifier();

    QTimer *idle_timer_;
    NeuralNetwork nn_;
    Agent ag_;

    double target_x_, target_y_, target_z_;

    double distance_;

public slots:
    void update();
    void updateSubstep(const bool print = false);

private:
	Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *agent_entity_;
    Qt3DCore::QEntity *target_entity_;
	Qt3DCore::QEntity *ground_entity_;
    
public:
    Qt3DCore::QTransform *agent_transform_;
    Qt3DCore::QTransform *target_transform_;
	Qt3DCore::QTransform *ground_transform_;

    void moveTarget();
};
