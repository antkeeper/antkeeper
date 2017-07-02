/*
 * Copyright (C) 2017  Christopher J. Howard
 *
 * This file is part of Antkeeper Source Code.
 *
 * Antkeeper Source Code is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Antkeeper Source Code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Antkeeper Source Code.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "title-state.hpp"
#include "main-menu-state.hpp"
#include "../application.hpp"
#include "../camera-controller.hpp"
#include <iostream>
#include <SDL.h>

const float blankDuration = 0.0f;
const float fadeInDuration = 0.5f;
const float hangDuration = 1.0f;
const float fadeOutDuration = 0.5f;
const float titleDelay = 2.0f;
const float copyrightDelay = 3.0f;
const float pressAnyKeyDelay = 5.0f;

TitleState::TitleState(Application* application):
	ApplicationState(application)
{}

TitleState::~TitleState()
{}

void TitleState::enter()
{	
	// Setup screen fade-in transition
	fadeIn = false;
	fadeOut = false;
	
	// BG
	application->bgBatch.resize(1);
	BillboardBatch::Range* bgRange = application->bgBatch.addRange();
	bgRange->start = 0;
	bgRange->length = 1;
	Billboard* bgBillboard = application->bgBatch.getBillboard(0);
	bgBillboard->setDimensions(Vector2(1.0f, 1.0f));
	bgBillboard->setTranslation(Vector3(0.5f, 0.5f, 0.0f));
	bgBillboard->setTintColor(Vector4(1, 1, 1, 1));
	application->bgBatch.update();
	
	application->vignettePass.setRenderTarget(&application->defaultRenderTarget);
	application->bgCompositor.addPass(&application->vignettePass);
	application->bgCompositor.load(nullptr);
	application->bgCamera.setOrthographic(0, 1.0f, 1.0f, 0, -1.0f, 1.0f);
	application->bgCamera.lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	application->bgCamera.setCompositor(&application->bgCompositor);
	application->bgCamera.setCompositeIndex(0);
	
	application->backgroundLayer->addObject(&application->bgCamera);
	application->backgroundLayer->addObject(&application->bgBatch);
	
	// Title ant hill
	application->defaultLayer->addObject(&application->antHillModelInstance);
	
	// Setup lighting
	application->sunlight.setColor(glm::vec3(1.0f));
	application->sunlight.setDirection(glm::normalize(glm::vec3(0.5, -1, -0.5)));
	
	// Setup soil pass
	application->soilPass.setRenderTarget(&application->defaultRenderTarget);
	application->defaultCompositor.addPass(&application->soilPass);
	
	// Create terrain
	application->terrain.create(255, 255, Vector3(50, 20, 50));
	application->terrain.getSurfaceModel()->getGroup(0)->material = application->materialLoader->load("data/materials/debug-terrain-surface.mtl");
	terrainSurface.setModel(application->terrain.getSurfaceModel());
	terrainSurface.setTranslation(Vector3(0, 0, 0));
	terrainSubsurface.setModel(application->terrain.getSubsurfaceModel());
	terrainSubsurface.setTranslation(Vector3(0, 0, 0));
	//application->defaultLayer->addObject(&terrainSurface);
	//application->defaultLayer->addObject(&terrainSubsurface);
	navmesh = application->terrain.getSurfaceNavmesh();
	
	// Load level
	application->loadLevel();
	
	// Setup lighting pass
	application->lightingPass.setRenderTarget(&application->defaultRenderTarget);
	application->lightingPass.setShadowMap(0);
	application->lightingPass.setShadowCamera(&application->camera);
	application->lightingPass.setModelLoader(application->modelLoader);
	application->defaultCompositor.addPass(&application->lightingPass);
	
	application->camera.lookAt(
		glm::vec3(0.0f, 0.0f, 10.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
	application->camera.setCompositor(&application->defaultCompositor);
	application->camera.setCompositeIndex(0);
	
	// Setup scene
	application->defaultLayer->addObject(&application->camera);
	
	// Load compositor
	RenderQueue renderQueue;
	const std::list<SceneObject*>* objects = application->defaultLayer->getObjects();
	for (const SceneObject* object: *objects)
		renderQueue.queue(object);
	RenderContext renderContext;
	renderContext.camera = nullptr;
	renderContext.layer = application->defaultLayer;
	renderContext.queue = &renderQueue;
	application->defaultCompositor.load(&renderContext);
	
	application->inputManager->addWindowObserver(this);
	windowResized(application->width, application->height);
	
	// Setup camera controller
	application->surfaceCam->setCamera(&application->camera);
	application->surfaceCam->setFocalPoint(Vector3(0.0f));
	application->surfaceCam->setFocalDistance(50.0f);
	application->surfaceCam->setElevation(glm::radians(0.0f));
	application->surfaceCam->setAzimuth(glm::radians(0.0f));
	application->surfaceCam->setTargetFocalPoint(application->surfaceCam->getFocalPoint());
	application->surfaceCam->setTargetFocalDistance(application->surfaceCam->getFocalDistance());
	application->surfaceCam->setTargetElevation(application->surfaceCam->getElevation());
	application->surfaceCam->setTargetAzimuth(application->surfaceCam->getAzimuth());
	application->surfaceCam->update(0.0f);
	
	// Setup fade-in
	application->blackoutImage->setVisible(true);
	application->fadeInTween->start();
	
	// Start timer
	stateTime = 0.0f;
	substate = 0;
}

void TitleState::execute()
{
	// Add dt to state time
	stateTime += application->dt;
	
	if (substate == 0 || substate == 1)
	{
		if (stateTime >= titleDelay && !application->titleImage->isVisible())
		{
			application->titleImage->setVisible(true);
			application->titleFadeInTween->start();
		}
		
		if (stateTime >= pressAnyKeyDelay && !application->anyKeyLabel->isVisible())
		{
			application->anyKeyLabel->setVisible(true);
			application->anyKeyFadeInTween->start();
		}
	}
	
	if (substate == 0 && stateTime >= titleDelay && application->titleFadeInTween->isStopped())
	{
		substate = 1;
	}
	
	// Listen for fade-in skip and "press any key"
	if (substate < 2)
	{
		InputEvent event;
		application->inputManager->listen(&event);
		
		if (event.type != InputEvent::Type::NONE)
		{
			application->menuControlProfile->update();
			application->inputManager->update();
						
			// Check if application was closed
			if (application->escape.isTriggered())
			{
				application->close(EXIT_SUCCESS);
				return;
			}
			// Check if fullscreen was toggled
			else if (application->toggleFullscreen.isTriggered() && !application->toggleFullscreen.wasTriggered())
			{
				application->changeFullscreen();
			}
			else if (!application->menuCancel.isTriggered())
			{
				if (substate == 0)
				{
					// Remove fade-in
					substate = 1;
					application->fadeInTween->stop();
					application->blackoutImage->setTintColor(Vector4(0.0f));
					application->blackoutImage->setVisible(false);
					application->titleFadeInTween->stop();
					application->titleImage->setVisible(true);
					application->titleImage->setTintColor(Vector4(1.0f));
					application->anyKeyFadeInTween->start();
					application->anyKeyLabel->setVisible(true);
				}
				else if (substate == 1)
				{
					// Enter main menu
					substate = 2;
					application->titleFadeInTween->stop();
					application->titleFadeOutTween->start();
					application->anyKeyFadeInTween->stop();
					application->anyKeyFadeOutTween->stop();
					application->anyKeyLabel->setVisible(false);
					
					application->antHillZoomInTween->start();
					
					application->blackoutImage->setVisible(true);
					application->antHillFadeOutTween->start();
				}
			}
		}
	}
	
	application->surfaceCam->update(application->dt);
}

void TitleState::exit()
{
	// Remove objects from scene
	application->defaultLayer->removeObject(&application->antHillModelInstance);
	
	application->inputManager->removeWindowObserver(this);
}

void TitleState::windowClosed()
{
	application->close(EXIT_SUCCESS);
}

void TitleState::windowResized(int width, int height)
{
	// Update application dimensions
	application->width = width;
	application->height = height;
	if (application->fullscreen)
	{
		application->fullscreenWidth = width;
		application->fullscreenHeight = height;
	}
	else
	{
		application->windowedWidth = width;
		application->windowedHeight = height;
	}
	
	// Setup default render target
	application->defaultRenderTarget.width = application->width;
	application->defaultRenderTarget.height = application->height;
	
	// Resize UI
	application->resizeUI();
	
	// 3D camera
	application->camera.setPerspective(
		glm::radians(25.0f),
		(float)application->width / (float)application->height,
		0.1f,
		1000.0f);
}
