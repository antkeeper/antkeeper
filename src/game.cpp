/*
 * Copyright (C) 2017-2019  Christopher J. Howard
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

#include "game.hpp"
#include "states/game-state.hpp"
#include "states/splash-state.hpp"
#include "states/sandbox-state.hpp"
#include "paths.hpp"
#include "ui/ui.hpp"
#include "graphics/ui-render-pass.hpp"
#include "graphics/shadow-map-render-pass.hpp"
#include "graphics/clear-render-pass.hpp"
#include "graphics/sky-render-pass.hpp"
#include "graphics/lighting-render-pass.hpp"
#include "graphics/silhouette-render-pass.hpp"
#include "graphics/final-render-pass.hpp"
#include "resources/resource-manager.hpp"
#include "resources/text-file.hpp"
#include "game/camera-rig.hpp"
#include "game/lens.hpp"
#include "game/forceps.hpp"
#include "game/brush.hpp"
#include "entity/component-manager.hpp"
#include "entity/components/transform-component.hpp"
#include "entity/components/model-component.hpp"
#include "entity/entity-manager.hpp"
#include "entity/entity-template.hpp"
#include "entity/system-manager.hpp"
#include "entity/systems/sound-system.hpp"
#include "entity/systems/collision-system.hpp"
#include "entity/systems/render-system.hpp"
#include "entity/systems/tool-system.hpp"
#include "entity/systems/locomotion-system.hpp"
#include "entity/systems/behavior-system.hpp"
#include "entity/systems/steering-system.hpp"
#include "entity/systems/particle-system.hpp"
#include "stb/stb_image_write.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <thread>

Game::Game(int argc, char* argv[]):
	currentState(nullptr),
	window(nullptr)
{
	// Get paths
	dataPath = getDataPath();
	configPath = getConfigPath();

	// Create config path if it doesn't exist
	if (!pathExists(configPath))
	{
		createDirectory(configPath);
	}

	std::cout << configPath << std::endl;

	// Setup resource manager
	resourceManager = new ResourceManager();
	resourceManager->include(dataPath);
	resourceManager->include(configPath);

	// Read strings file
	stringTable = resourceManager->load<CSVTable>("strings.csv");

	// Build string map
	for (int row = 0; row < stringTable->size(); ++row)
	{
		stringMap[(*stringTable)[row][0]] = row;
	}

	// Determine number of languages
	languageCount = (*stringTable)[0].size() - 1;

	// Set current language to English
	currentLanguage = 0;

	splashState = new SplashState(this);
	sandboxState = new SandboxState(this);
}

Game::~Game()
{
	if (window)
	{
		windowManager->destroyWindow(window);
	}
}

void Game::changeState(GameState* state)
{
	if (currentState != nullptr)
	{
		currentState->exit();
	}

	currentState = state;
	if (currentState != nullptr)
	{
		currentState->enter();
	}
}

std::string Game::getString(std::size_t languageIndex, const std::string& name) const
{
	std::string value;

	auto it = stringMap.find(name);
	if (it != stringMap.end())
	{
		value = (*stringTable)[it->second][languageIndex + 1];
		if (value.empty())
		{
			value = std::string("# EMPTY STRING: ") + name + std::string(" #");
		}
	}
	else
	{
		value = std::string("# MISSING STRING: ") + name + std::string(" #");
	}

	return value;
}

void Game::changeLanguage(std::size_t languageIndex)
{
	currentLanguage = languageIndex;
	window->setTitle(getString(getCurrentLanguage(), "title").c_str());

	restringUI();
	resizeUI(w, h);
}

void Game::toggleFullscreen()
{
	fullscreen = !fullscreen;
	window->setFullscreen(fullscreen);
}

void Game::setUpdateRate(double frequency)
{
	stepScheduler.setStepFrequency(frequency);
}

void Game::setup()
{
	// Initialize default parameters
	title = getString(currentLanguage, "title");
	float windowSizeRatio = 3.0f / 4.0f;
	const Display* display = deviceManager->getDisplays()->front();
	w = std::get<0>(display->getDimensions()) * windowSizeRatio;
	h = std::get<1>(display->getDimensions()) * windowSizeRatio;
	w = 1600;
	h = 900;
	int x = std::get<0>(display->getPosition()) + std::get<0>(display->getDimensions()) / 2 - w / 2;
	int y = std::get<1>(display->getPosition()) + std::get<1>(display->getDimensions()) / 2 - h / 2;
	unsigned int flags = WindowFlag::RESIZABLE;
	fullscreen = false;
	bool vsync = true;
	double maxFrameDuration = 0.25;
	double stepFrequency = 60.0;

	// Create window
	window = windowManager->createWindow(title.c_str(), x, y, w, h, fullscreen, flags);
	if (!window)
	{
		throw std::runtime_error("Game::Game(): Failed to create window.");
	}

	// Set v-sync mode
	window->setVSync(vsync);

	// Setup step scheduler
	stepScheduler.setMaxFrameDuration(maxFrameDuration);
	stepScheduler.setStepFrequency(stepFrequency);
	timestep = stepScheduler.getStepPeriod();

	// Setup performance sampling
	performanceSampler.setSampleSize(15);

	// Get DPI and font size
	dpi = display->getDPI();
	fontSizePT = 14;
	fontSizePX = fontSizePT * (1.0f / 72.0f) * dpi;

	// Create scene
	scene = new Scene(&stepInterpolator);
	
	// Setup control profile
	keyboard = deviceManager->getKeyboards()->front();
	mouse = deviceManager->getMice()->front();
	closeControl.bindKey(keyboard, Scancode::ESCAPE);
	closeControl.setActivatedCallback(std::bind(&Application::close, this, EXIT_SUCCESS));
	fullscreenControl.bindKey(keyboard, Scancode::F11);
	fullscreenControl.setActivatedCallback(std::bind(&Game::toggleFullscreen, this));
	openRadialMenuControl.bindKey(keyboard, Scancode::LSHIFT);
	moveForwardControl.bindKey(keyboard, Scancode::W);
	moveBackControl.bindKey(keyboard, Scancode::S);
	moveLeftControl.bindKey(keyboard, Scancode::A);
	moveRightControl.bindKey(keyboard, Scancode::D);
	//rotateCCWControl.bindKey(keyboard, Scancode::Q);
	//rotateCWControl.bindKey(keyboard, Scancode::E);
	moveRightControl.bindKey(keyboard, Scancode::D);
	zoomInControl.bindKey(keyboard, Scancode::EQUALS);
	zoomInControl.bindMouseWheelAxis(mouse, MouseWheelAxis::POSITIVE_Y);
	zoomOutControl.bindKey(keyboard, Scancode::MINUS);
	zoomOutControl.bindMouseWheelAxis(mouse, MouseWheelAxis::NEGATIVE_Y);
	adjustCameraControl.bindMouseButton(mouse, 2);
	dragCameraControl.bindMouseButton(mouse, 3);
	toggleNestViewControl.bindKey(keyboard, Scancode::N);
	toggleWireframeControl.bindKey(keyboard, Scancode::V);
	screenshotControl.bindKey(keyboard, Scancode::B);
	toggleEditModeControl.bindKey(keyboard, Scancode::TAB);
	controlProfile.registerControl("close", &closeControl);
	controlProfile.registerControl("fullscreen", &fullscreenControl);
	controlProfile.registerControl("open-radial-menu", &openRadialMenuControl);
	controlProfile.registerControl("move-forward", &moveForwardControl);
	controlProfile.registerControl("move-back", &moveBackControl);
	controlProfile.registerControl("move-left", &moveLeftControl);
	controlProfile.registerControl("move-right", &moveRightControl);
	controlProfile.registerControl("rotate-ccw", &rotateCCWControl);
	controlProfile.registerControl("rotate-cw", &rotateCWControl);
	controlProfile.registerControl("zoom-in", &zoomInControl);
	controlProfile.registerControl("zoom-out", &zoomOutControl);
	controlProfile.registerControl("adjust-camera", &adjustCameraControl);
	controlProfile.registerControl("drag-camera", &dragCameraControl);
	controlProfile.registerControl("toggle-nest-view", &toggleNestViewControl);
	controlProfile.registerControl("toggle-wireframe", &toggleWireframeControl);
	controlProfile.registerControl("screenshot", &screenshotControl);
	controlProfile.registerControl("toggle-edit-mode", &toggleEditModeControl);

	wireframe = false;
	toggleWireframeControl.setActivatedCallback(std::bind(&Game::toggleWireframe, this));
	screenshotControl.setActivatedCallback(std::bind(&Game::queueScreenshot, this));
	screenshotQueued = false;

	TestEvent event1, event2, event3;
	event1.id = 1;
	event2.id = 2;
	event3.id = 3;


	eventDispatcher.subscribe<TestEvent>(this);
	eventDispatcher.schedule(event1, 1.0);
	eventDispatcher.schedule(event2, 10.0);
	eventDispatcher.schedule(event3, 1.0);

	// Load model resources
	try
	{
		lensModel = resourceManager->load<Model>("lens.mdl");
		forcepsModel = resourceManager->load<Model>("forceps.mdl");
		brushModel = resourceManager->load<Model>("brush.mdl");
		smokeMaterial = resourceManager->load<Material>("smoke.mtl");
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to load one or more models: \"" << e.what() << "\"" << std::endl;
		close(EXIT_FAILURE);
	}

	try
	{
		splashTexture = resourceManager->load<Texture2D>("epigraph.png");
		hudSpriteSheetTexture = resourceManager->load<Texture2D>("hud.png");

		// Read texture atlas file
		CSVTable* atlasTable = resourceManager->load<CSVTable>("hud-atlas.csv");

		// Build texture atlas
		for (int row = 0; row < atlasTable->size(); ++row)
		{
			std::stringstream ss;
			float x;
			float y;
			float w;
			float h;

			ss << (*atlasTable)[row][1];
			ss >> x;
			ss.str(std::string());
			ss.clear();
			ss << (*atlasTable)[row][2];
			ss >> y;
			ss.str(std::string());
			ss.clear();
			ss << (*atlasTable)[row][3];
			ss >> w;
			ss.str(std::string());
			ss.clear();
			ss << (*atlasTable)[row][4];
			ss >> h;
			ss.str(std::string());

			y = static_cast<float>(hudSpriteSheetTexture->getHeight()) - y - h;
			x = (int)(x + 0.5f);
			y = (int)(y + 0.5f);
			w = (int)(w + 0.5f);
			h = (int)(h + 0.5f);

			hudTextureAtlas.insert((*atlasTable)[row][0], Rect(Vector2(x, y), Vector2(x + w, y + h)));
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to load one or more textures: \"" << e.what() << "\"" << std::endl;
		close(EXIT_FAILURE);
	}

	// Load font resources
	try
	{
		//labelTypeface = resourceManager->load<Typeface>("open-sans-regular.ttf");
		labelTypeface = resourceManager->load<Typeface>("caveat-bold.ttf");
		labelFont = labelTypeface->createFont(fontSizePX);

		debugTypeface = resourceManager->load<Typeface>("inconsolata-bold.ttf");
		debugFont = debugTypeface->createFont(fontSizePX);
		debugTypeface->loadCharset(debugFont, UnicodeRange::BASIC_LATIN);


		std::set<char32_t> charset;
		charset.emplace(U'方');
		charset.emplace(U'蕴');
		labelTypeface->loadCharset(labelFont, UnicodeRange::BASIC_LATIN);
		labelTypeface->loadCharset(labelFont, charset);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to load one or more fonts: \"" << e.what() << "\"" << std::endl;
		close(EXIT_FAILURE);
	}


	Shader* shader = resourceManager->load<Shader>("depth-pass.glsl");

	/*
	VertexFormat format;
	format.addAttribute<Vector4>(0);
	format.addAttribute<Vector3>(1);
	format.addAttribute<Vector3>(2);

	VertexBuffer* vb = graphicsContext->createVertexBuffer(format);
	vb->resize(1000);
	vb->setData(bla, 0, 1000);

	IndexBuffer* ib = graphicsContext->createIndexBuffer();
	ib->resize(300);
	ib->setData(bla, 0, 300);

	graphicsContext->bind(framebuffer);
	graphicsContext->bind(shader);
	graphicsContext->bind(vb);
	graphicsContext->bind(ib);
	graphicsContext->draw(100, TRIANGLES);
	*/

	cameraRig = nullptr;
	orbitCam = new OrbitCam();
	orbitCam->attachCamera(&camera);
	freeCam = new FreeCam();
	freeCam->attachCamera(&camera);

	silhouetteRenderTarget.width = w;
	silhouetteRenderTarget.height = h;

	// Silhouette framebuffer texture
	glGenTextures(1, &silhouetteRenderTarget.texture);
	glBindTexture(GL_TEXTURE_2D, silhouetteRenderTarget.texture);
	glTexImage2D(GL_TEXTURE_2D, 0,  GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Generate framebuffer
	glGenFramebuffers(1, &silhouetteRenderTarget.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, silhouetteRenderTarget.framebuffer);

	// Attach textures to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, silhouetteRenderTarget.texture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_NONE);
	
	// Unbind framebuffer and texture
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Setup rendering
	defaultRenderTarget.width = w;
	defaultRenderTarget.height = h;
	defaultRenderTarget.framebuffer = 0;
	clearPass = new ClearRenderPass();
	clearPass->setRenderTarget(&defaultRenderTarget);
	clearPass->setClear(true, true, false);
	clearPass->setClearColor(Vector4(0.0f));
	clearPass->setClearDepth(1.0f);
	skyPass = new SkyRenderPass(resourceManager);
	skyPass->setRenderTarget(&defaultRenderTarget);
	uiPass = new UIRenderPass(resourceManager);
	uiPass->setRenderTarget(&defaultRenderTarget);
	uiCompositor.addPass(uiPass);
	uiCompositor.load(nullptr);

	// Setup UI batching
	uiBatch = new BillboardBatch();
	uiBatch->resize(1024);
	uiBatcher = new UIBatcher();

	// Setup root UI element
	uiRootElement = new UIContainer();
	eventDispatcher.subscribe<MouseMovedEvent>(uiRootElement);
	eventDispatcher.subscribe<MouseButtonPressedEvent>(uiRootElement);
	eventDispatcher.subscribe<MouseButtonReleasedEvent>(uiRootElement);

	// Create splash screen background element
	splashBackgroundImage = new UIImage();
	splashBackgroundImage->setLayerOffset(-1);
	splashBackgroundImage->setTintColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	splashBackgroundImage->setVisible(false);
	uiRootElement->addChild(splashBackgroundImage);

	// Create splash screen element
	splashImage = new UIImage();
	splashImage->setTexture(splashTexture);
	splashImage->setVisible(false);
	uiRootElement->addChild(splashImage);

	Rect hudTextureAtlasBounds(Vector2(0), Vector2(hudSpriteSheetTexture->getWidth(), hudSpriteSheetTexture->getHeight()));
	auto normalizeTextureBounds = [](const Rect& texture, const Rect& atlas)
	{
		Vector2 atlasDimensions = Vector2(atlas.getWidth(), atlas.getHeight());
		return Rect(texture.getMin() / atlasDimensions, texture.getMax() / atlasDimensions);
	};

	// Create HUD elements
	hudContainer = new UIContainer();
	hudContainer->setVisible(false);
	uiRootElement->addChild(hudContainer);
	

	toolIndicatorBGImage = new UIImage();
	toolIndicatorBGImage->setTexture(hudSpriteSheetTexture);
	toolIndicatorBGImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator"), hudTextureAtlasBounds));
	hudContainer->addChild(toolIndicatorBGImage);

	toolIndicatorsBounds = new Rect[8];
	toolIndicatorsBounds[0] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator-brush"), hudTextureAtlasBounds);
	toolIndicatorsBounds[1] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator-spade"), hudTextureAtlasBounds);
	toolIndicatorsBounds[2] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator-lens"), hudTextureAtlasBounds);
	toolIndicatorsBounds[3] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator-test-tube"), hudTextureAtlasBounds);
	toolIndicatorsBounds[4] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator-forceps"), hudTextureAtlasBounds);
	toolIndicatorsBounds[5] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator"), hudTextureAtlasBounds);
	toolIndicatorsBounds[6] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator"), hudTextureAtlasBounds);
	toolIndicatorsBounds[7] = normalizeTextureBounds(hudTextureAtlas.getBounds("tool-indicator"), hudTextureAtlasBounds);

	toolIndicatorIconImage = new UIImage();
	toolIndicatorIconImage->setTexture(hudSpriteSheetTexture);
	toolIndicatorIconImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-icon-brush"), hudTextureAtlasBounds));
	toolIndicatorBGImage->addChild(toolIndicatorIconImage);

	buttonContainer = new UIContainer();
	hudContainer->addChild(buttonContainer);

	playButtonBGImage = new UIImage();
	playButtonBGImage->setTexture(hudSpriteSheetTexture);
	playButtonBGImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("button-background"), hudTextureAtlasBounds));
	//buttonContainer->addChild(playButtonBGImage);

	pauseButtonBGImage = new UIImage();
	pauseButtonBGImage->setTexture(hudSpriteSheetTexture);
	pauseButtonBGImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("button-background"), hudTextureAtlasBounds));
	//buttonContainer->addChild(pauseButtonBGImage);

	fastForwardButtonBGImage = new UIImage();
	fastForwardButtonBGImage->setTexture(hudSpriteSheetTexture);
	fastForwardButtonBGImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("button-background"), hudTextureAtlasBounds));
	//buttonContainer->addChild(fastForwardButtonBGImage);

	playButtonImage = new UIImage();
	playButtonImage->setTexture(hudSpriteSheetTexture);
	playButtonImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("button-play"), hudTextureAtlasBounds));
	//buttonContainer->addChild(playButtonImage);

	fastForwardButtonImage = new UIImage();
	fastForwardButtonImage->setTexture(hudSpriteSheetTexture);
	fastForwardButtonImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("button-fast-forward-2x"), hudTextureAtlasBounds));
	//buttonContainer->addChild(fastForwardButtonImage);

	pauseButtonImage = new UIImage();
	pauseButtonImage->setTexture(hudSpriteSheetTexture);
	pauseButtonImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("button-pause"), hudTextureAtlasBounds));
	//buttonContainer->addChild(pauseButtonImage);

	radialMenuContainer = new UIContainer();
	radialMenuContainer->setVisible(false);
	uiRootElement->addChild(radialMenuContainer);

	radialMenuBackgroundImage = new UIImage();
	radialMenuBackgroundImage->setTintColor(Vector4(Vector3(0.0f), 0.25f));
	radialMenuContainer->addChild(radialMenuBackgroundImage);

	radialMenuImage = new UIImage();
	radialMenuImage->setTexture(hudSpriteSheetTexture);
	radialMenuImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("radial-menu"), hudTextureAtlasBounds));
	radialMenuContainer->addChild(radialMenuImage);

	radialMenuSelectorImage = new UIImage();
	radialMenuSelectorImage->setTexture(hudSpriteSheetTexture);
	radialMenuSelectorImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("radial-menu-selector"), hudTextureAtlasBounds));
	radialMenuContainer->addChild(radialMenuSelectorImage);

	toolIconBrushImage = new UIImage();
	toolIconBrushImage->setTexture(hudSpriteSheetTexture);
	toolIconBrushImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-icon-brush"), hudTextureAtlasBounds));
	radialMenuImage->addChild(toolIconBrushImage);

	toolIconLensImage = new UIImage();
	toolIconLensImage->setTexture(hudSpriteSheetTexture);
	toolIconLensImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-icon-lens"), hudTextureAtlasBounds));
	radialMenuImage->addChild(toolIconLensImage);

	toolIconForcepsImage = new UIImage();
	toolIconForcepsImage->setTexture(hudSpriteSheetTexture);
	toolIconForcepsImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-icon-forceps"), hudTextureAtlasBounds));
	radialMenuImage->addChild(toolIconForcepsImage);

	toolIconSpadeImage = new UIImage();
	toolIconSpadeImage->setTexture(hudSpriteSheetTexture);
	toolIconSpadeImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-icon-spade"), hudTextureAtlasBounds));
	//radialMenuImage->addChild(toolIconSpadeImage);

	toolIconCameraImage = new UIImage();
	toolIconCameraImage->setTexture(hudSpriteSheetTexture);
	toolIconCameraImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-icon-camera"), hudTextureAtlasBounds));
	radialMenuImage->addChild(toolIconCameraImage);

	toolIconTestTubeImage = new UIImage();
	toolIconTestTubeImage->setTexture(hudSpriteSheetTexture);
	toolIconTestTubeImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("tool-icon-test-tube"), hudTextureAtlasBounds));
	//radialMenuImage->addChild(toolIconTestTubeImage);


	antTag = new UIContainer();
	antTag->setLayerOffset(-10);
	antTag->setVisible(false);

	uiRootElement->addChild(antTag);

	antLabelContainer = new UIContainer();
	antTag->addChild(antLabelContainer);

	antLabelTL = new UIImage();
	antLabelTR = new UIImage();
	antLabelBL = new UIImage();
	antLabelBR = new UIImage();
	antLabelCC = new UIImage();
	antLabelCT = new UIImage();
	antLabelCB = new UIImage();
	antLabelCL = new UIImage();
	antLabelCR = new UIImage();

	antLabelTL->setTexture(hudSpriteSheetTexture);
	antLabelTR->setTexture(hudSpriteSheetTexture);
	antLabelBL->setTexture(hudSpriteSheetTexture);
	antLabelBR->setTexture(hudSpriteSheetTexture);
	antLabelCC->setTexture(hudSpriteSheetTexture);
	antLabelCT->setTexture(hudSpriteSheetTexture);
	antLabelCB->setTexture(hudSpriteSheetTexture);
	antLabelCL->setTexture(hudSpriteSheetTexture);
	antLabelCR->setTexture(hudSpriteSheetTexture);

	Rect labelTLBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-tl"), hudTextureAtlasBounds);
	Rect labelTRBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-tr"), hudTextureAtlasBounds);
	Rect labelBLBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-bl"), hudTextureAtlasBounds);
	Rect labelBRBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-br"), hudTextureAtlasBounds);
	Rect labelCCBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-cc"), hudTextureAtlasBounds);
	Rect labelCTBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-ct"), hudTextureAtlasBounds);
	Rect labelCBBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-cb"), hudTextureAtlasBounds);
	Rect labelCLBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-cl"), hudTextureAtlasBounds);
	Rect labelCRBounds = normalizeTextureBounds(hudTextureAtlas.getBounds("label-cr"), hudTextureAtlasBounds);

	Vector2 labelTLMin = labelTLBounds.getMin();
	Vector2 labelTRMin = labelTRBounds.getMin();
	Vector2 labelBLMin = labelBLBounds.getMin();
	Vector2 labelBRMin = labelBRBounds.getMin();
	Vector2 labelCCMin = labelCCBounds.getMin();
	Vector2 labelCTMin = labelCTBounds.getMin();
	Vector2 labelCBMin = labelCBBounds.getMin();
	Vector2 labelCLMin = labelCLBounds.getMin();
	Vector2 labelCRMin = labelCRBounds.getMin();
	Vector2 labelTLMax = labelTLBounds.getMax();
	Vector2 labelTRMax = labelTRBounds.getMax();
	Vector2 labelBLMax = labelBLBounds.getMax();
	Vector2 labelBRMax = labelBRBounds.getMax();
	Vector2 labelCCMax = labelCCBounds.getMax();
	Vector2 labelCTMax = labelCTBounds.getMax();
	Vector2 labelCBMax = labelCBBounds.getMax();
	Vector2 labelCLMax = labelCLBounds.getMax();
	Vector2 labelCRMax = labelCRBounds.getMax();

	antLabelTL->setTextureBounds(labelTLBounds);
	antLabelTR->setTextureBounds(labelTRBounds);
	antLabelBL->setTextureBounds(labelBLBounds);
	antLabelBR->setTextureBounds(labelBRBounds);
	antLabelCC->setTextureBounds(labelCCBounds);
	antLabelCT->setTextureBounds(labelCTBounds);
	antLabelCB->setTextureBounds(labelCBBounds);
	antLabelCL->setTextureBounds(labelCLBounds);
	antLabelCR->setTextureBounds(labelCRBounds);

	antLabelContainer->addChild(antLabelTL);
	antLabelContainer->addChild(antLabelTR);
	antLabelContainer->addChild(antLabelBL);
	antLabelContainer->addChild(antLabelBR);
	antLabelContainer->addChild(antLabelCC);
	antLabelContainer->addChild(antLabelCT);
	antLabelContainer->addChild(antLabelCB);
	antLabelContainer->addChild(antLabelCL);
	antLabelContainer->addChild(antLabelCR);

	antLabel = new UILabel();
	antLabel->setFont(labelFont);
	antLabel->setText("Boggy B.");
	antLabel->setTintColor(Vector4(Vector3(0.0f), 1.0f));
	antLabel->setLayerOffset(1);
	antLabelContainer->addChild(antLabel);

	fpsLabel = new UILabel();
	fpsLabel->setFont(debugFont);
	fpsLabel->setTintColor(Vector4(1, 1, 0, 1));
	fpsLabel->setLayerOffset(50);
	fpsLabel->setAnchor(Anchor::TOP_LEFT);
	uiRootElement->addChild(fpsLabel);


	antPin = new UIImage();
	antPin->setTexture(hudSpriteSheetTexture);
	antPin->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("label-pin"), hudTextureAtlasBounds));
	antTag->addChild(antPin);

	antLabelPinHole = new UIImage();
	antLabelPinHole->setTexture(hudSpriteSheetTexture);
	antLabelPinHole->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("label-pin-hole"), hudTextureAtlasBounds));
	antLabelContainer->addChild(antLabelPinHole);

	notificationBoxImage = new UIImage();
	notificationBoxImage->setTexture(hudSpriteSheetTexture);
	notificationBoxImage->setTextureBounds(normalizeTextureBounds(hudTextureAtlas.getBounds("notification-box"), hudTextureAtlasBounds));
	notificationBoxImage->setVisible(false);
	hudContainer->addChild(notificationBoxImage);

	// Construct show notification animation clip
	notificationCount = 0;
	AnimationChannel<Vector2>* channel2;
	showNotificationClip.setInterpolator(easeOutQuint<Vector2>);
	channel2 = showNotificationClip.addChannel(0);
	channel2->insertKeyframe(0.0f, Vector2(0.0f, 0.0f));
	channel2->insertKeyframe(0.5f, Vector2(32.0f, 1.0f));
	showNotificationAnimation.setClip(&showNotificationClip);
	showNotificationAnimation.setSpeed(1.0f);
	showNotificationAnimation.setTimeFrame(showNotificationClip.getTimeFrame());
	animator.addAnimation(&showNotificationAnimation);

	showNotificationAnimation.setAnimateCallback
	(
		[this](std::size_t id, const Vector2& values)
		{
			notificationBoxImage->setVisible(true);
			notificationBoxImage->setTintColor(Vector4(Vector3(1.0f), values.y));
			//notificationBoxImage->setTranslation(Vector2(0.0f, -32.0f + values.x));
		}
	);
	showNotificationAnimation.setEndCallback
	(
		[this]()
		{
			hideNotificationAnimation.rewind();
			hideNotificationAnimation.play();
		}
	);

	// Construct hide notification animation clip
	hideNotificationClip.setInterpolator(easeOutQuint<float>);
	AnimationChannel<float>* channel;
	channel = hideNotificationClip.addChannel(0);
	channel->insertKeyframe(0.0f, 1.0f);
	channel->insertKeyframe(10.5f, 1.0f);
	channel->insertKeyframe(12.0f, 0.0f);
	hideNotificationAnimation.setClip(&hideNotificationClip);
	hideNotificationAnimation.setSpeed(1.0f);
	hideNotificationAnimation.setTimeFrame(hideNotificationClip.getTimeFrame());
	animator.addAnimation(&hideNotificationAnimation);
	hideNotificationAnimation.setAnimateCallback
	(
		[this](std::size_t id, float opacity)
		{
			notificationBoxImage->setTintColor(Vector4(Vector3(1.0f), opacity));
		}
	);
	hideNotificationAnimation.setEndCallback
	(
		[this]()
		{
			notificationBoxImage->setVisible(false);
			--notificationCount;
			popNotification();
		}
	);

	// Construct box selection
	boxSelectionImageBackground = new UIImage();
	boxSelectionImageBackground->setAnchor(Anchor::CENTER);
	boxSelectionImageTop = new UIImage();
	boxSelectionImageTop->setAnchor(Anchor::TOP_LEFT);
	boxSelectionImageBottom = new UIImage();
	boxSelectionImageBottom->setAnchor(Anchor::BOTTOM_LEFT);
	boxSelectionImageLeft = new UIImage();
	boxSelectionImageLeft->setAnchor(Anchor::TOP_LEFT);
	boxSelectionImageRight = new UIImage();
	boxSelectionImageRight->setAnchor(Anchor::TOP_RIGHT);
	boxSelectionContainer = new UIContainer();
	boxSelectionContainer->setLayerOffset(80);
	boxSelectionContainer->addChild(boxSelectionImageBackground);
	boxSelectionContainer->addChild(boxSelectionImageTop);
	boxSelectionContainer->addChild(boxSelectionImageBottom);
	boxSelectionContainer->addChild(boxSelectionImageLeft);
	boxSelectionContainer->addChild(boxSelectionImageRight);
	boxSelectionContainer->setVisible(false);
	uiRootElement->addChild(boxSelectionContainer);
	boxSelectionImageBackground->setTintColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	boxSelectionContainer->setTintColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	boxSelectionBorderWidth = 2.0f;

	cameraGridColor = Vector4(1, 1, 1, 0.5f);
	cameraGridY0Image = new UIImage();
	cameraGridY0Image->setAnchor(Vector2(0.5f, (1.0f / 3.0f)));
	cameraGridY0Image->setTintColor(cameraGridColor);
	cameraGridY1Image = new UIImage();
	cameraGridY1Image->setAnchor(Vector2(0.5f, (2.0f / 3.0f)));
	cameraGridY1Image->setTintColor(cameraGridColor);
	cameraGridX0Image = new UIImage();
	cameraGridX0Image->setAnchor(Vector2((1.0f / 3.0f), 0.5f));
	cameraGridX0Image->setTintColor(cameraGridColor);
	cameraGridX1Image = new UIImage();
	cameraGridX1Image->setAnchor(Vector2((2.0f / 3.0f), 0.5f));
	cameraGridX1Image->setTintColor(cameraGridColor);
	cameraGridContainer = new UIContainer();
	cameraGridContainer->addChild(cameraGridY0Image);
	cameraGridContainer->addChild(cameraGridY1Image);
	cameraGridContainer->addChild(cameraGridX0Image);
	cameraGridContainer->addChild(cameraGridX1Image);
	cameraGridContainer->setVisible(true);
	uiRootElement->addChild(cameraGridContainer);

	cameraFlashImage = new UIImage();
	cameraFlashImage->setLayerOffset(99);
	cameraFlashImage->setTintColor(Vector4(1.0f));
	cameraFlashImage->setVisible(false);
	uiRootElement->addChild(cameraFlashImage);

	blackoutImage = new UIImage();
	blackoutImage->setLayerOffset(98);
	blackoutImage->setTintColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	blackoutImage->setVisible(false);
	uiRootElement->addChild(blackoutImage);

	// Construct fade-in animation clip
	fadeInClip.setInterpolator(easeOutCubic<float>);
	channel = fadeInClip.addChannel(0);
	channel->insertKeyframe(0.0f, 1.0f);
	channel->insertKeyframe(1.0f, 0.0f);

	// Construct fade-out animation clip
	fadeOutClip.setInterpolator(easeOutCubic<float>);
	channel = fadeOutClip.addChannel(0);
	channel->insertKeyframe(0.0f, 0.0f);
	channel->insertKeyframe(1.0f, 1.0f);

	// Setup fade-in animation callbacks
	fadeInAnimation.setAnimateCallback
	(
		[this](std::size_t id, float opacity)
		{
			Vector3 color = Vector3(blackoutImage->getTintColor());
			blackoutImage->setTintColor(Vector4(color, opacity));
		}
	);
	fadeInAnimation.setEndCallback
	(
		[this]()
		{
			blackoutImage->setVisible(false);
			if (fadeInEndCallback != nullptr)
			{
				fadeInEndCallback();
			}
		}
	);

	// Setup fade-out animation callbacks
	fadeOutAnimation.setAnimateCallback
	(
		[this](std::size_t id, float opacity)
		{
			Vector3 color = Vector3(blackoutImage->getTintColor());
			blackoutImage->setTintColor(Vector4(color, opacity));
		}
	);
	fadeOutAnimation.setEndCallback
	(
		[this]()
		{
			blackoutImage->setVisible(false);
			if (fadeOutEndCallback != nullptr)
			{
				fadeOutEndCallback();
			}
		}
	);

	animator.addAnimation(&fadeInAnimation);
	animator.addAnimation(&fadeOutAnimation);


	// Construct camera flash animation clip
	cameraFlashClip.setInterpolator(easeOutQuad<float>);
	channel = cameraFlashClip.addChannel(0);
	channel->insertKeyframe(0.0f, 1.0f);
	channel->insertKeyframe(1.0f, 0.0f);

	// Setup camera flash animation
	float flashDuration = 0.5f;
	cameraFlashAnimation.setSpeed(1.0f / flashDuration);
	cameraFlashAnimation.setLoop(false);
	cameraFlashAnimation.setClip(&cameraFlashClip);
	cameraFlashAnimation.setTimeFrame(cameraFlashClip.getTimeFrame());
	cameraFlashAnimation.setAnimateCallback
	(
		[this](std::size_t id, float opacity)
		{
			cameraFlashImage->setTintColor(Vector4(Vector3(1.0f), opacity));
		}
	);
	cameraFlashAnimation.setStartCallback
	(
		[this]()
		{
			cameraFlashImage->setVisible(true);
			cameraFlashImage->setTintColor(Vector4(1.0f));
			cameraFlashImage->resetTweens();
		}
	);
	cameraFlashAnimation.setEndCallback
	(
		[this]()
		{
			cameraFlashImage->setVisible(false);
		}
	);
	animator.addAnimation(&cameraFlashAnimation);
	
	// Setup default compositor
	{
		defaultCompositor.load(nullptr);
	}

	// Setup shadow map pass and compositor
	{
		// Set shadow map resolution
		shadowMapResolution = 4096;
		
		// Generate shadow map framebuffer
		glGenFramebuffers(1, &shadowMapFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);

		// Generate shadow map depth texture
		glGenTextures(1, &shadowMapDepthTextureID);
		glBindTexture(GL_TEXTURE_2D, shadowMapDepthTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		
		// Attach depth texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapDepthTextureID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		
		// Unbind shadow map depth texture
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// Setup shadow map render target
		shadowMapRenderTarget.width = shadowMapResolution;
		shadowMapRenderTarget.height = shadowMapResolution;
		shadowMapRenderTarget.framebuffer = shadowMapFramebuffer;
		
		// Setup texture class
		shadowMapDepthTexture.setTextureID(shadowMapDepthTextureID);
		shadowMapDepthTexture.setWidth(shadowMapResolution);
		shadowMapDepthTexture.setHeight(shadowMapResolution);
		
		// Setup shadow map render pass
		shadowMapPass = new ShadowMapRenderPass(resourceManager);
		shadowMapPass->setRenderTarget(&shadowMapRenderTarget);
		shadowMapPass->setViewCamera(&camera);
		shadowMapPass->setLightCamera(&sunlightCamera);
		
		// Setup shadow map compositor
		shadowMapCompositor.addPass(shadowMapPass);
		shadowMapCompositor.load(nullptr);
	}

	// Setup scene
	{
		defaultLayer = scene->addLayer();

		// Setup lighting pass
		lightingPass = new LightingRenderPass(resourceManager);
		lightingPass->setRenderTarget(&defaultRenderTarget);
		lightingPass->setShadowMapPass(shadowMapPass);
		lightingPass->setShadowMap(&shadowMapDepthTexture);

		// Setup clear silhouette pass
		clearSilhouettePass = new ClearRenderPass();
		clearSilhouettePass->setRenderTarget(&silhouetteRenderTarget);
		clearSilhouettePass->setClear(true, false, false);
		clearSilhouettePass->setClearColor(Vector4(0.0f));

		// Setup silhouette pass
		silhouettePass = new SilhouetteRenderPass(resourceManager);
		silhouettePass->setRenderTarget(&silhouetteRenderTarget);

		// Setup final pass
		finalPass = new FinalRenderPass(resourceManager);
		finalPass->setRenderTarget(&defaultRenderTarget);
		finalPass->setSilhouetteRenderTarget(&silhouetteRenderTarget);

		// Setup default compositor
		defaultCompositor.addPass(clearPass);
		defaultCompositor.addPass(skyPass);
		defaultCompositor.addPass(lightingPass);
		defaultCompositor.addPass(clearSilhouettePass);
		defaultCompositor.addPass(silhouettePass);
		defaultCompositor.addPass(finalPass);
		defaultCompositor.load(nullptr);

		// Setup camera
		camera.setPerspective(glm::radians(40.0f), static_cast<float>(w) / static_cast<float>(h), 0.1, 100.0f);
		camera.lookAt(Vector3(0.0f, 4.0f, 2.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
		camera.setCompositor(&defaultCompositor);
		camera.setCompositeIndex(1);
		defaultLayer->addObject(&camera);

		// Setup sun
		sunlight.setDirection(Vector3(0, -1, 0));
		setTimeOfDay(11.0f);
		defaultLayer->addObject(&sunlight);

		// Setup sunlight camera
		sunlightCamera.setOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
		sunlightCamera.setCompositor(&shadowMapCompositor);
		sunlightCamera.setCompositeIndex(0);
		sunlightCamera.setCullingEnabled(true);
		sunlightCamera.setCullingMask(&camera.getViewFrustum());
		defaultLayer->addObject(&sunlightCamera);
	}
	
	// Setup UI scene
	uiLayer = scene->addLayer();
	uiLayer->addObject(uiBatch);
	uiLayer->addObject(&uiCamera);

	// Setup UI camera
	uiCamera.lookAt(Vector3(0), Vector3(0, 0, -1), Vector3(0, 1, 0));
	uiCamera.resetTweens();
	uiCamera.setCompositor(&uiCompositor);
	uiCamera.setCompositeIndex(0);
	uiCamera.setCullingEnabled(false);
	
	restringUI();
	resizeUI(w, h);


	time = 0.0f;
	

	// Tools
	currentTool = nullptr;

	lens = new Lens(lensModel, &animator);
	lens->setOrbitCam(orbitCam);
	defaultLayer->addObject(lens->getModelInstance());
	defaultLayer->addObject(lens->getSpotlight());
	lens->setSunDirection(-sunlightCamera.getForward());

	ModelInstance* modelInstance = lens->getModelInstance();
	for (std::size_t i = 0; i < modelInstance->getModel()->getGroupCount(); ++i)
	{
		Material* material = modelInstance->getModel()->getGroup(i)->material->clone();
		material->setFlags(material->getFlags() | 256);
		modelInstance->setMaterialSlot(i, material);
	}

	// Forceps
	forceps = new Forceps(forcepsModel, &animator);
	forceps->setOrbitCam(orbitCam);
	defaultLayer->addObject(forceps->getModelInstance());

	// Brush
	brush = new Brush(brushModel, &animator);
	brush->setOrbitCam(orbitCam);
	defaultLayer->addObject(brush->getModelInstance());

	glEnable(GL_MULTISAMPLE);
	//
	
	performanceSampler.setSampleSize(30);

	// Initialize component manager
	componentManager = new ComponentManager();

	// Initialize entity manager
	entityManager = new EntityManager(componentManager);

	// Initialize systems
	soundSystem = new SoundSystem(componentManager);
	collisionSystem = new CollisionSystem(componentManager);
	renderSystem = new RenderSystem(componentManager, defaultLayer);
	toolSystem = new ToolSystem(componentManager);
	toolSystem->setPickingCamera(&camera);
	toolSystem->setPickingViewport(Vector4(0, 0, w, h));
	eventDispatcher.subscribe<MouseMovedEvent>(toolSystem);
	behaviorSystem = new BehaviorSystem(componentManager);
	steeringSystem = new SteeringSystem(componentManager);
	locomotionSystem = new LocomotionSystem(componentManager);
	particleSystem = new ParticleSystem(componentManager);
	particleSystem->resize(1000);
	particleSystem->setMaterial(smokeMaterial);
	particleSystem->setDirection(Vector3(0, 1, 0));
	lens->setParticleSystem(particleSystem);
	particleSystem->getBillboardBatch()->setAlignment(&camera, BillboardAlignmentMode::SPHERICAL);
	defaultLayer->addObject(particleSystem->getBillboardBatch());


	// Initialize system manager
	systemManager = new SystemManager();
	systemManager->addSystem(soundSystem);
	systemManager->addSystem(behaviorSystem);
	systemManager->addSystem(steeringSystem);
	systemManager->addSystem(locomotionSystem);
	systemManager->addSystem(collisionSystem);
	systemManager->addSystem(toolSystem);
	systemManager->addSystem(particleSystem);
	systemManager->addSystem(renderSystem);

	EntityID sidewalkPanel;
	sidewalkPanel = createInstanceOf("sidewalk-panel");

	EntityID antHill = createInstanceOf("ant-hill");
	setTranslation(antHill, Vector3(20, 0, 40));

	EntityID antNest = createInstanceOf("ant-nest");
	setTranslation(antNest, Vector3(20, 0, 40));

	lollipop = createInstanceOf("lollipop");
	setTranslation(lollipop, Vector3(30.0f, 3.5f * 0.5f, -30.0f));
	setRotation(lollipop, glm::angleAxis(glm::radians(8.85f), Vector3(1.0f, 0.0f, 0.0f)));

	// Load navmesh
	TriangleMesh* navmesh = resourceManager->load<TriangleMesh>("sidewalk.mesh");

	// Find surface
	TriangleMesh::Triangle* surface = nullptr;
	Vector3 barycentricPosition;
	Ray ray;
	ray.origin = Vector3(0, 100, 0);
	ray.direction = Vector3(0, -1, 0);
	auto intersection = ray.intersects(*navmesh);
	if (std::get<0>(intersection))
	{
		surface = (*navmesh->getTriangles())[std::get<3>(intersection)];

		Vector3 position = ray.extrapolate(std::get<1>(intersection));
		Vector3 a = surface->edge->vertex->position;
		Vector3 b = surface->edge->next->vertex->position;
		Vector3 c = surface->edge->previous->vertex->position;

		barycentricPosition = barycentric(position, a, b, c);
	}

	for (int i = 0; i < 0; ++i)
	{
		EntityID ant = createInstanceOf("worker-ant");
		setTranslation(ant, Vector3(0.0f, 0, 0.0f));

		BehaviorComponent* behavior = new BehaviorComponent();
		SteeringComponent* steering = new SteeringComponent();
		LeggedLocomotionComponent* locomotion = new LeggedLocomotionComponent();
		componentManager->addComponent(ant, behavior);
		componentManager->addComponent(ant, steering);
		componentManager->addComponent(ant, locomotion);

		locomotion->surface = surface;
		behavior->wanderTriangle = surface;
		locomotion->barycentricPosition = barycentricPosition;
	}

	
	EntityID tool0 = createInstanceOf("lens");

	changeState(splashState);
}

void Game::input()
{
}

void Game::update(float t, float dt)
{
	this->time = t;

	// Dispatch scheduled events
	eventDispatcher.update(t);

	// Execute current state
	if (currentState != nullptr)
	{
		currentState->execute();
	}

	// Update systems
	systemManager->update(t, dt);

	// Update animations
	animator.animate(dt);

	std::stringstream stream;
	stream.precision(2);
	stream << std::fixed << (performanceSampler.getMeanFrameDuration() * 1000.0f);
	fpsLabel->setText(stream.str());

	uiRootElement->update();

	controlProfile.update();
}

void Game::render()
{
	// Perform sub-frame interpolation on UI elements
	uiRootElement->interpolate(stepScheduler.getScheduledSubsteps());

	// Update and batch UI elements
	uiBatcher->batch(uiBatch, uiRootElement);

	// Perform sub-frame interpolation particles
	particleSystem->getBillboardBatch()->interpolate(stepScheduler.getScheduledSubsteps());
	particleSystem->getBillboardBatch()->batch();

	// Render scene
	renderer.render(*scene);

	// Swap window framebuffers
	window->swapBuffers();

	if (screenshotQueued)
	{
		screenshot();
		screenshotQueued = false;
	}
}

void Game::exit()
{

}

void Game::handleEvent(const WindowResizedEvent& event)
{
	w = event.width;
	h = event.height;

	defaultRenderTarget.width = event.width;
	defaultRenderTarget.height = event.height;
	glViewport(0, 0, event.width, event.height);


	camera.setPerspective(glm::radians(40.0f), static_cast<float>(w) / static_cast<float>(h), 0.1, 100.0f);


	toolSystem->setPickingViewport(Vector4(0, 0, w, h));

	resizeUI(event.width, event.height);
}

void Game::handleEvent(const KeyPressedEvent& event)
{
	if (event.scancode == Scancode::SPACE)
	{
		changeLanguage((getCurrentLanguage() + 1) % getLanguageCount());
	}
}

void Game::handleEvent(const TestEvent& event)
{
	std::cout << "Event received!!! ID: " << event.id << std::endl;
}

void Game::resizeUI(int w, int h)
{
	// Adjust root element dimensions
	uiRootElement->setDimensions(Vector2(w, h));
	uiRootElement->update();

	splashBackgroundImage->setDimensions(Vector2(w, h));
	splashBackgroundImage->setAnchor(Anchor::TOP_LEFT);


	// Resize splash screen image
	splashImage->setAnchor(Anchor::CENTER);
	splashImage->setDimensions(Vector2(splashTexture->getWidth(), splashTexture->getHeight()));

	// Adjust UI camera projection matrix
	uiCamera.setOrthographic(0.0f, w, h, 0.0f, -1.0f, 1.0f);
	uiCamera.resetTweens();

	// Resize camera flash image
	cameraFlashImage->setDimensions(Vector2(w, h));
	cameraFlashImage->setAnchor(Anchor::CENTER);

	// Resize blackout image
	blackoutImage->setDimensions(Vector2(w, h));
	blackoutImage->setAnchor(Anchor::CENTER);

	// Resize HUD
	float hudPadding = 20.0f;
	hudContainer->setDimensions(Vector2(w - hudPadding * 2.0f, h - hudPadding * 2.0f));
	hudContainer->setAnchor(Anchor::CENTER);

	// Tool indicator
	Rect toolIndicatorBounds = hudTextureAtlas.getBounds("tool-indicator");
	toolIndicatorBGImage->setDimensions(Vector2(toolIndicatorBounds.getWidth(), toolIndicatorBounds.getHeight()));
	toolIndicatorBGImage->setAnchor(Anchor::TOP_LEFT);

	Rect toolIndicatorIconBounds = hudTextureAtlas.getBounds("tool-indicator-lens");
	toolIndicatorIconImage->setDimensions(Vector2(toolIndicatorIconBounds.getWidth(), toolIndicatorIconBounds.getHeight()));
	toolIndicatorIconImage->setAnchor(Anchor::CENTER);

	
	// Buttons
	Rect playButtonBounds = hudTextureAtlas.getBounds("button-play");
	Rect fastForwardButtonBounds = hudTextureAtlas.getBounds("button-fast-forward-2x");
	Rect pauseButtonBounds = hudTextureAtlas.getBounds("button-pause");
	Rect buttonBackgroundBounds = hudTextureAtlas.getBounds("button-background");
	Vector2 buttonBGDimensions = Vector2(buttonBackgroundBounds.getWidth(), buttonBackgroundBounds.getHeight());
	float buttonMargin = 10.0f;
	float buttonDepth = 15.0f;

	float buttonContainerWidth = fastForwardButtonBounds.getWidth();
	float buttonContainerHeight = fastForwardButtonBounds.getHeight();
	buttonContainer->setDimensions(Vector2(buttonContainerWidth, buttonContainerHeight));
	buttonContainer->setAnchor(Anchor::TOP_RIGHT);

	playButtonImage->setDimensions(Vector2(playButtonBounds.getWidth(), playButtonBounds.getHeight()));
	playButtonImage->setAnchor(Vector2(0.0f, 0.0f));
	playButtonBGImage->setDimensions(buttonBGDimensions);
	playButtonBGImage->setAnchor(Vector2(0.0f, 1.0f));

	fastForwardButtonImage->setDimensions(Vector2(fastForwardButtonBounds.getWidth(), fastForwardButtonBounds.getHeight()));
	fastForwardButtonImage->setAnchor(Vector2(0.5f, 5.0f));
	fastForwardButtonBGImage->setDimensions(buttonBGDimensions);
	fastForwardButtonBGImage->setAnchor(Vector2(0.5f, 0.5f));

	pauseButtonImage->setDimensions(Vector2(pauseButtonBounds.getWidth(), pauseButtonBounds.getHeight()));
	pauseButtonImage->setAnchor(Vector2(1.0f, 0.0f));
	pauseButtonBGImage->setDimensions(buttonBGDimensions);
	pauseButtonBGImage->setAnchor(Vector2(1.0f, 1.0f));

	// Radial menu
	Rect radialMenuBounds = hudTextureAtlas.getBounds("radial-menu");
	radialMenuContainer->setDimensions(Vector2(w, h));
	radialMenuContainer->setAnchor(Anchor::CENTER);
	radialMenuContainer->setLayerOffset(30);

	radialMenuBackgroundImage->setDimensions(Vector2(w, h));
	radialMenuBackgroundImage->setAnchor(Anchor::CENTER);
	radialMenuBackgroundImage->setLayerOffset(-1);

	//radialMenuImage->setDimensions(Vector2(w * 0.5f, h * 0.5f));
	radialMenuImage->setDimensions(Vector2(radialMenuBounds.getWidth(), radialMenuBounds.getHeight()));
	radialMenuImage->setAnchor(Anchor::CENTER);

	Rect radialMenuSelectorBounds = hudTextureAtlas.getBounds("radial-menu-selector");
	radialMenuSelectorImage->setDimensions(Vector2(radialMenuSelectorBounds.getWidth(), radialMenuSelectorBounds.getHeight()));
	radialMenuSelectorImage->setAnchor(Anchor::CENTER);

	Rect toolIconBrushBounds = hudTextureAtlas.getBounds("tool-icon-brush");
	toolIconBrushImage->setDimensions(Vector2(toolIconBrushBounds.getWidth(), toolIconBrushBounds.getHeight()));
	toolIconBrushImage->setAnchor(Anchor::CENTER);

	Rect toolIconLensBounds = hudTextureAtlas.getBounds("tool-icon-lens");
	toolIconLensImage->setDimensions(Vector2(toolIconLensBounds.getWidth(), toolIconLensBounds.getHeight()));
	toolIconLensImage->setAnchor(Anchor::CENTER);

	Rect toolIconForcepsBounds = hudTextureAtlas.getBounds("tool-icon-forceps");
	toolIconForcepsImage->setDimensions(Vector2(toolIconForcepsBounds.getWidth(), toolIconForcepsBounds.getHeight()));
	toolIconForcepsImage->setAnchor(Anchor::CENTER);

	Rect toolIconSpadeBounds = hudTextureAtlas.getBounds("tool-icon-spade");
	toolIconSpadeImage->setDimensions(Vector2(toolIconSpadeBounds.getWidth(), toolIconSpadeBounds.getHeight()));
	toolIconSpadeImage->setAnchor(Anchor::CENTER);

	Rect toolIconCameraBounds = hudTextureAtlas.getBounds("tool-icon-camera");
	toolIconCameraImage->setDimensions(Vector2(toolIconCameraBounds.getWidth(), toolIconCameraBounds.getHeight()));
	toolIconCameraImage->setAnchor(Anchor::CENTER);


	Rect toolIconTestTubeBounds = hudTextureAtlas.getBounds("tool-icon-test-tube");
	toolIconTestTubeImage->setDimensions(Vector2(toolIconTestTubeBounds.getWidth(), toolIconTestTubeBounds.getHeight()));
	toolIconTestTubeImage->setAnchor(Anchor::CENTER);

	Rect labelCornerBounds = hudTextureAtlas.getBounds("label-tl");
	Vector2 labelCornerDimensions(labelCornerBounds.getWidth(), labelCornerBounds.getHeight());

	Vector2 antLabelPadding(10.0f, 6.0f);
	antLabelContainer->setDimensions(antLabel->getDimensions() + antLabelPadding * 2.0f);
	antLabelContainer->setTranslation(Vector2(0.0f, (int)(-antPin->getDimensions().y * 0.125f)));
	antLabelTL->setDimensions(labelCornerDimensions);
	antLabelTR->setDimensions(labelCornerDimensions);
	antLabelBL->setDimensions(labelCornerDimensions);
	antLabelBR->setDimensions(labelCornerDimensions);
	antLabelCC->setDimensions(Vector2(antLabel->getDimensions().x - labelCornerDimensions.x * 2.0f + antLabelPadding.x * 2.0f, antLabel->getDimensions().y - labelCornerDimensions.y * 2.0f + antLabelPadding.y * 2.0f));
	antLabelCT->setDimensions(Vector2(antLabel->getDimensions().x - labelCornerDimensions.x * 2.0f + antLabelPadding.x * 2.0f, labelCornerDimensions.y));
	antLabelCB->setDimensions(Vector2(antLabel->getDimensions().x - labelCornerDimensions.x * 2.0f + antLabelPadding.x * 2.0f, labelCornerDimensions.y));

	antLabelCL->setDimensions(Vector2(labelCornerDimensions.x, antLabel->getDimensions().y - labelCornerDimensions.y * 2.0f + antLabelPadding.y * 2.0f));
	antLabelCR->setDimensions(Vector2(labelCornerDimensions.x, antLabel->getDimensions().y - labelCornerDimensions.y * 2.0f + antLabelPadding.y * 2.0f));



	antLabelContainer->setAnchor(Vector2(0.5f, 0.5f));
	antLabelTL->setAnchor(Anchor::TOP_LEFT);
	antLabelTR->setAnchor(Anchor::TOP_RIGHT);
	antLabelBL->setAnchor(Anchor::BOTTOM_LEFT);
	antLabelBR->setAnchor(Anchor::BOTTOM_RIGHT);
	antLabelCC->setAnchor(Anchor::CENTER);
	antLabelCT->setAnchor(Vector2(0.5f, 0.0f));
	antLabelCB->setAnchor(Vector2(0.5f, 1.0f));
	antLabelCL->setAnchor(Vector2(0.0f, 0.5f));
	antLabelCR->setAnchor(Vector2(1.0f, 0.5f));
	antLabel->setAnchor(Anchor::CENTER);

	Rect antPinBounds = hudTextureAtlas.getBounds("label-pin");
	antPin->setDimensions(Vector2(antPinBounds.getWidth(), antPinBounds.getHeight()));
	antPin->setAnchor(Vector2(0.5f, 1.0f));

	Rect pinHoleBounds = hudTextureAtlas.getBounds("label-pin-hole");
	antLabelPinHole->setDimensions(Vector2(pinHoleBounds.getWidth(), pinHoleBounds.getHeight()));
	antLabelPinHole->setAnchor(Vector2(0.5f, 0.0f));
	antLabelPinHole->setTranslation(Vector2(0.0f, -antLabelPinHole->getDimensions().y * 0.5f));
	antLabelPinHole->setLayerOffset(2);


	float pinDistance = 20.0f;
	antTag->setAnchor(Anchor::CENTER);
	antTag->setDimensions(Vector2(antLabelContainer->getDimensions().x, antPin->getDimensions().y));

	Rect notificationBoxBounds = hudTextureAtlas.getBounds("notification-box");
	notificationBoxImage->setDimensions(Vector2(notificationBoxBounds.getWidth(), notificationBoxBounds.getHeight()));
	notificationBoxImage->setAnchor(Vector2(0.5f, 0.0f));

	float cameraGridLineWidth = 2.0f;
	cameraGridContainer->setDimensions(Vector2(w, h));
	cameraGridY0Image->setDimensions(Vector2(w, cameraGridLineWidth));
	cameraGridY1Image->setDimensions(Vector2(w, cameraGridLineWidth));
	cameraGridX0Image->setDimensions(Vector2(cameraGridLineWidth, h));
	cameraGridX1Image->setDimensions(Vector2(cameraGridLineWidth, h));
	cameraGridY0Image->setTranslation(Vector2(0));
	cameraGridY1Image->setTranslation(Vector2(0));
	cameraGridX0Image->setTranslation(Vector2(0));
	cameraGridX1Image->setTranslation(Vector2(0));

	UIImage* icons[] =
	{
		toolIconBrushImage,
		nullptr,
		toolIconLensImage,
		nullptr,
		toolIconForcepsImage,
		nullptr,
		toolIconCameraImage,
		nullptr
	};

	Rect radialMenuIconRingBounds = hudTextureAtlas.getBounds("radial-menu-icon-ring");
	float iconOffset = radialMenuIconRingBounds.getWidth() * 0.5f;
	float sectorAngle = (2.0f * 3.14159264f) / 8.0f;
	for (int i = 0; i < 8; ++i)
	{
		float angle = sectorAngle * static_cast<float>(i - 4);
		Vector2 translation = Vector2(std::cos(angle), std::sin(angle)) * iconOffset;
		translation.x = (int)(translation.x + 0.5f);
		translation.y = (int)(translation.y + 0.5f);

		if (icons[i] != nullptr)
		{
			icons[i]->setTranslation(translation);
		}
	}
}

void Game::restringUI()
{

}

void Game::setTimeOfDay(float time)
{
	Vector3 midnight = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 sunrise = Vector3(-1.0f, 0.0f, 0.0f);
	Vector3 noon = Vector3(0, -1.0f, 0.0f);
	Vector3 sunset = Vector3(1.0f, 0.0f, 0.0f);

	float angles[4] =
	{
		glm::radians(270.0f), // 00:00
		glm::radians(0.0f),   // 06:00
		glm::radians(90.0f),  // 12:00
		glm::radians(180.0f)  // 18:00
	};

	int index0 = static_cast<int>(fmod(time, 24.0f) / 6.0f);
	int index1 = (index0 + 1) % 4;

	float t = (time - (static_cast<float>(index0) * 6.0f)) / 6.0f;

	Quaternion rotation0 = glm::angleAxis(angles[index0], Vector3(1, 0, 0));
	Quaternion rotation1 = glm::angleAxis(angles[index1], Vector3(1, 0, 0));
	Quaternion rotation = glm::normalize(glm::slerp(rotation0, rotation1, t));

	Vector3 direction = glm::normalize(rotation * Vector3(0, 0, 1));

	sunlight.setDirection(direction);

	Vector3 up = glm::normalize(rotation * Vector3(0, 1, 0));

	sunlightCamera.lookAt(Vector3(0, 0, 0), sunlight.getDirection(), up);
}

void Game::toggleWireframe()
{
	wireframe = !wireframe;

	float width = (wireframe) ? 1.0f : 0.0f;
	lightingPass->setWireframeLineWidth(width);
}

void Game::queueScreenshot()
{
	screenshotQueued = true;
	cameraFlashImage->setVisible(false);
	cameraGridContainer->setVisible(false);

	soundSystem->scrot();
}

void Game::screenshot()
{
	screenshotQueued = false;

	// Read pixel data from framebuffer
	unsigned char* pixels = new unsigned char[w * h * 3];
	glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Get game title in current language
	std::string title = getString(getCurrentLanguage(), "title");

	// Convert title to lowercase
	std::transform(title.begin(), title.end(), title.begin(), ::tolower);

	// Get system time
	auto now = std::chrono::system_clock::now();
	std::time_t tt = std::chrono::system_clock::to_time_t(now);
	std::size_t ms = (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000).count();

	// Create screenshot directory if it doesn't exist
	std::string screenshotDirectory = configPath + std::string("/screenshots/");
	if (!pathExists(screenshotDirectory))
	{
		createDirectory(screenshotDirectory);
	}

	// Build screenshot file name
	std::stringstream stream;
	stream << screenshotDirectory;
	stream << title;
	stream << std::put_time(std::localtime(&tt), "-%Y%m%d-%H%M%S-");
	stream << std::setfill('0') << std::setw(3) << ms;
	stream << ".png";
	std::string filename = stream.str();

	// Write screenshot to file in separate thread
	std::thread screenshotThread(Game::saveScreenshot, filename, w, h, pixels);
	screenshotThread.detach();

	// Play camera flash animation
	cameraFlashAnimation.stop();
	cameraFlashAnimation.rewind();
	cameraFlashAnimation.play();

	// Play camera shutter sound
	
	// Restore camera UI visibility
	cameraGridContainer->setVisible(true);
}

void Game::boxSelect(float x, float y, float w, float h)
{
	boxSelectionContainer->setTranslation(Vector2(x, y));
	boxSelectionContainer->setDimensions(Vector2(w, h));
	boxSelectionImageBackground->setDimensions(Vector2(w, h));
	boxSelectionImageTop->setDimensions(Vector2(w, boxSelectionBorderWidth));
	boxSelectionImageBottom->setDimensions(Vector2(w, boxSelectionBorderWidth));
	boxSelectionImageLeft->setDimensions(Vector2(boxSelectionBorderWidth, h));
	boxSelectionImageRight->setDimensions(Vector2(boxSelectionBorderWidth, h));
	boxSelectionContainer->setVisible(true);
}

void Game::fadeIn(float duration, const Vector3& color, std::function<void()> callback)
{
	if (fadeInAnimation.isPlaying())
	{
		return;
	}

	fadeOutAnimation.stop();
	this->fadeInEndCallback = callback;
	blackoutImage->setTintColor(Vector4(color, 1.0f));
	blackoutImage->setVisible(true);
	fadeInAnimation.setSpeed(1.0f / duration);
	fadeInAnimation.setLoop(false);
	fadeInAnimation.setClip(&fadeInClip);
	fadeInAnimation.setTimeFrame(fadeInClip.getTimeFrame());
	fadeInAnimation.rewind();
	fadeInAnimation.play();

	blackoutImage->resetTweens();
	uiRootElement->update();
}

void Game::fadeOut(float duration, const Vector3& color, std::function<void()> callback)
{
	if (fadeOutAnimation.isPlaying())
	{
		return;
	}

	fadeInAnimation.stop();
	this->fadeOutEndCallback = callback;
	blackoutImage->setVisible(true);
	blackoutImage->setTintColor(Vector4(color, 0.0f));
	fadeOutAnimation.setSpeed(1.0f / duration);
	fadeOutAnimation.setLoop(false);
	fadeOutAnimation.setClip(&fadeOutClip);
	fadeOutAnimation.setTimeFrame(fadeOutClip.getTimeFrame());
	fadeOutAnimation.rewind();
	fadeOutAnimation.play();

	blackoutImage->resetTweens();
	uiRootElement->update();
}

void Game::selectTool(int toolIndex)
{
	Tool* tools[] =
	{
		brush,
		nullptr,
		lens,
		nullptr,
		forceps,
		nullptr,
		nullptr,
		nullptr
	};

	Tool* nextTool = tools[toolIndex];
	if (nextTool != currentTool)
	{
		if (currentTool)
		{
			currentTool->setActive(false);
			currentTool->update(0.0f);
		}

		currentTool = nextTool;
		if (currentTool)
		{
			currentTool->setActive(true);
		}
		else
		{
			pushNotification("Invalid tool.");
		}
	}

	if (1)
	{
		toolIndicatorIconImage->setTextureBounds(toolIndicatorsBounds[toolIndex]);
		toolIndicatorIconImage->setVisible(true);
	}
	else
	{
		toolIndicatorIconImage->setVisible(false);
	}
}

void Game::pushNotification(const std::string& text)
{
	std::cout << text << std::endl;
	++notificationCount;

	if (notificationCount == 1)
	{
		popNotification();
	}
}

void Game::popNotification()
{
	if (notificationCount > 0)
	{
		showNotificationAnimation.rewind();
		showNotificationAnimation.play();
	}
}

EntityID Game::createInstanceOf(const std::string& templateName)
{

	EntityTemplate* entityTemplate = resourceManager->load<EntityTemplate>(templateName + ".ent");

	EntityID entity = entityManager->createEntity();
	entityTemplate->apply(entity, componentManager);

	return entity;
}

void Game::destroyInstance(EntityID entity)
{
	entityManager->destroyEntity(entity);
}

void Game::setTranslation(EntityID entity, const Vector3& translation)
{
	TransformComponent* component = static_cast<TransformComponent*>(componentManager->getComponent(entity, ComponentType::TRANSFORM));
	if (!component)
	{
		return;
	}

	component->transform.translation = translation;
}

void Game::setRotation(EntityID entity, const Quaternion& rotation)
{
	TransformComponent* component = static_cast<TransformComponent*>(componentManager->getComponent(entity, ComponentType::TRANSFORM));
	if (!component)
	{
		return;
	}

	component->transform.rotation = rotation;
}

void Game::setScale(EntityID entity, const Vector3& scale)
{
	TransformComponent* component = static_cast<TransformComponent*>(componentManager->getComponent(entity, ComponentType::TRANSFORM));
	if (!component)
	{
		return;
	}

	component->transform.scale = scale;
}


void Game::saveScreenshot(const std::string& filename, unsigned int width, unsigned int height, unsigned char* pixels)
{
	stbi_flip_vertically_on_write(1);
	stbi_write_png(filename.c_str(), width, height, 3, pixels, width * 3);

	delete[] pixels;
}