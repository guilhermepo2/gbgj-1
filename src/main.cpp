#include <gueepo2d.h>
static gueepo::Shader* spriteShader;
static gueepo::SpriteBatcher* batch;

static gueepo::Texture* allSprites;

static gueepo::TextureRegion* ourHero;
static gueepo::math::vec2 ourHeroPosition;
static bool isHeroFacingLeft = true;

static gueepo::TextureRegion* groundTexture;
static gueepo::TextureRegion* baseGroundTexture;

// props
static gueepo::TextureRegion* resourceRock;
static gueepo::TextureRegion* monsterSprite;

static const int MAP_WIDTH = 4;
static const int MAP_HEIGHT = 4;
static const int TILE_SIZE = 64;

static const int TEXTURE_SIZE = 64;

// fonts
static gueepo::Shader* fontShader;
static gueepo::SpriteBatcher* fontBatcher;
static gueepo::Font* dogica;
static gueepo::FontSprite* dogicaFont;

// gameplay loop stuff?!
static int currentWave = 0;
static int inventoryCount = 0;
static int resourcesOnBaseCount = 0;
static int maxInventoryCount = 5;
static gueepo::string feedbackText = "";
static float textCount = 0.0f;
static float timeToTextDisappear = 5.0f;

struct Tile {
public:
	int x;
	int y;
	bool isBase;
	bool hasResource;
};

static std::vector<Tile> theMap;

static std::vector<gueepo::TextureRegion*> ourHeroIdleAnimation;
static int animCurrentFrame = 0;
static float animTimePerFrame = 0.2f;
static float animTileElapsed = 0.0f;

bool IsPlayerOnBase() {
	for (int i = 0; i < theMap.size(); i++) {
		if (theMap[i].x == ourHeroPosition.x && theMap[i].y == ourHeroPosition.y) {
			return theMap[i].isBase;
		}
	}

	return false;
}

void SetBaseOnPosition(int x, int y) {
	for (int i = 0; i < theMap.size(); i++) {
		if (theMap[i].x == x && theMap[i].y == y) {
			theMap[i].isBase = true;
			return;
		}
	}
}

void AssignResourceToRandomTile() {
	int randomIndex = gueepo::rand::Int() % theMap.size();
	int tries = 0;
	while (tries < 10 && theMap[randomIndex].isBase) {
		randomIndex = gueepo::rand::Int() % theMap.size();
		tries++;
	}

	if (!theMap[randomIndex].isBase) { // extremely unlikely, but hey, doesn't hurt to be safe
		theMap[randomIndex].hasResource = true;
	}
}

bool DoesTileHaveAResource(int x, int y) {
	for (int i = 0; i < theMap.size(); i++) {
		if (theMap[i].x == x && theMap[i].y == y) {
			return theMap[i].hasResource;
		}
	}

	return false;
}

bool IsPositionValid(int x, int y) {
	if(x >= -MAP_WIDTH && x <= MAP_WIDTH && y >= -MAP_HEIGHT && y <= MAP_HEIGHT) {
		return true;
	}

	return false;
}

class GameLayer : public gueepo::Layer {
public:
	GameLayer() : Layer("game layer") {}

	void OnAttach() override {
		GUEEPO2D_SCOPED_TIMER("game layer OnAttach");
		gueepo::Renderer::Initialize();
		gueepo::rand::Init();

		spriteShader = gueepo::Shader::CreateFromFile("./assets/shaders/sprite.vertex", "./assets/shaders/sprite.fragment");
		fontShader = gueepo::Shader::CreateFromFile("./assets/shaders/font.vertex", "./assets/shaders/font.fragment");

		batch = new gueepo::SpriteBatcher();
		batch->Initialize(gueepo::Renderer::GetRendererAPI(), spriteShader);

		fontBatcher = new gueepo::SpriteBatcher();
		fontBatcher->Initialize(gueepo::Renderer::GetRendererAPI(), fontShader);

		allSprites = gueepo::Texture::Create("./assets/sprites.png");
		ourHero = new gueepo::TextureRegion(allSprites, 0, 0, 32, 32);
		ourHeroIdleAnimation.push_back(new gueepo::TextureRegion(allSprites, 0, 0, 32, 32));
		ourHeroIdleAnimation.push_back(new gueepo::TextureRegion(allSprites, 32, 0, 32, 32));
		ourHeroIdleAnimation.push_back(new gueepo::TextureRegion(allSprites, 64, 0, 32, 32));
		ourHeroIdleAnimation.push_back(new gueepo::TextureRegion(allSprites, 96, 0, 32, 32));
		ourHeroIdleAnimation.push_back(new gueepo::TextureRegion(allSprites, 128, 0, 32, 32));

		groundTexture = new gueepo::TextureRegion(allSprites, 0, 32, 32, 32);
		baseGroundTexture = new gueepo::TextureRegion(allSprites, 32, 32, 32, 32);
		resourceRock = new gueepo::TextureRegion(allSprites, 0, 64, 32, 32);
		monsterSprite = new gueepo::TextureRegion(allSprites, 0, 96, 32, 32);

		ourHeroPosition.x = 0;
		ourHeroPosition.y = 0;

		for (int i = -MAP_HEIGHT; i <= MAP_HEIGHT; i++) {
			for (int j = -MAP_WIDTH; j <= MAP_WIDTH; j++) {
				Tile t;
				t.x = j;
				t.y = i;
				t.isBase = false;
				t.hasResource = false;
				theMap.push_back(t);
			}
		}

		SetBaseOnPosition(3, 4);
		SetBaseOnPosition(4, 4);
		SetBaseOnPosition(4, 3);
		SetBaseOnPosition(3, 3);

		m_Camera = std::make_unique<gueepo::OrtographicCamera>(1280, 720);
		m_Camera->SetBackgroundColor(0.1f, 0.13f, 0.13f, 1.0f);

		dogica = gueepo::Font::CreateFont("./assets/fonts/dogica.ttf");
		dogicaFont = new gueepo::FontSprite(dogica, 12);
		dogicaFont->SetLineGap(4.0f);

		// get some tiles and make them have treasures
		for (int i = 0; i < 3; i++) {
			AssignResourceToRandomTile();
		}

		// add 1 or 2 monsters?
	}

	void OnDetach() override {}
	
	void OnUpdate(float DeltaTime) override {
		if (animTileElapsed > animTimePerFrame) {
			animCurrentFrame = (animCurrentFrame + 1) % ourHeroIdleAnimation.size();
			animTileElapsed = 0.0f;
		}

		if (textCount < 0.0f) {
			feedbackText = "";
		}

		animTileElapsed += DeltaTime;
		textCount -= DeltaTime;
	}

	void OnInput(const gueepo::InputState& currentInputState) override {
		bool playerMoved = false;
		gueepo::math::vec2 oldPosition = ourHeroPosition;

		if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_D)) {
			ourHeroPosition.x += 1;
			isHeroFacingLeft = false;
			playerMoved = true;
		}
		else if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_A)) {
			ourHeroPosition.x -= 1;
			isHeroFacingLeft = true;
			playerMoved = true;
		}
		else if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_W)) {
			ourHeroPosition.y += 1;
			playerMoved = true;
		}
		else if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_S)) {
			ourHeroPosition.y -= 1;
			playerMoved = true;
		}

		if(!IsPositionValid(ourHeroPosition.x, ourHeroPosition.y)) {
			ourHeroPosition = oldPosition;
			playerMoved = false;
		}

		if (playerMoved) {
			if (DoesTileHaveAResource(ourHeroPosition.x, ourHeroPosition.y)) { // todo: make the player NOT move...
				ourHeroPosition = oldPosition;
				
				if (inventoryCount >= maxInventoryCount) {
					feedbackText = "You have plenty of resources!";
				}
				else {
					inventoryCount++;
					feedbackText = "You got some resources!";
				}

				textCount = timeToTextDisappear;
			}

			if (IsPlayerOnBase()) {
				if (inventoryCount > maxInventoryCount) {
					currentWave++;
				}
				resourcesOnBaseCount += inventoryCount;
				// todo: there's no easy way to append an integer to the feedback text here...
				// todo: maybe I can bake that into the string function, like feedbackText.appendNumber(int n)
				// todo: I *could* make a function that convert numbers to their utf8 and append *THAT*.
				feedbackText = "You've dropped all your resources on the base!";
				inventoryCount = 0;
			}
		}
	}
	void OnEvent(gueepo::Event& e) override {}


	void OnRender() override {
		gueepo::Color bgColor = m_Camera->GetBackGroundColor();
		gueepo::Renderer::Clear(bgColor.rgba[0], bgColor.rgba[1], bgColor.rgba[2], bgColor.rgba[3]);

		batch->Begin(*m_Camera);

		for (int i = 0; i < theMap.size(); i++) {
			if (theMap[i].isBase) {
				batch->Draw(baseGroundTexture, theMap[i].x * TILE_SIZE, theMap[i].y * TILE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE);
			}
			else if(!IsPlayerOnBase()) {
				batch->Draw(groundTexture, theMap[i].x * TILE_SIZE, theMap[i].y * TILE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE);

				if (theMap[i].hasResource) {
					batch->Draw(resourceRock, theMap[i].x * TILE_SIZE, theMap[i].y * TILE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE);
				}
			}
		}

		int xScaleModifier = 1;
		if(!isHeroFacingLeft) {
			xScaleModifier = -1;
		}
		batch->Draw(
			ourHeroIdleAnimation[animCurrentFrame], ourHeroPosition.x * TILE_SIZE, ourHeroPosition.y * TILE_SIZE,
			xScaleModifier * TEXTURE_SIZE, TEXTURE_SIZE
		);

		batch->End();

		fontBatcher->Begin(*m_Camera);
		fontBatcher->DrawText(dogicaFont, feedbackText, gueepo::math::vec2(-600.0f, -320.0f), 1.0f, gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f));
		fontBatcher->End();
	}
	void OnImGuiRender() override {}
private:
	std::unique_ptr<gueepo::OrtographicCamera> m_Camera;
};

class GBGJ1 : public gueepo::Application {
public:
	GBGJ1() : Application("goblin heck", 1280, 720) { PushLayer(new GameLayer()); }
	~GBGJ1() {}
};

gueepo::Application* gueepo::CreateApplication() {
	return new GBGJ1();
}
