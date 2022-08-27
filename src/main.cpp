#include <gueepo2d.h>
static gueepo::Shader* spriteShader;
static gueepo::SpriteBatcher* batch;

class GameLayer : public gueepo::Layer {
public:
	GameLayer() : Layer("game layer") {}

	void OnAttach() override {
		GUEEPO2D_SCOPED_TIMER("game layer OnAttach");
		gueepo::Renderer::Initialize();

		spriteShader = gueepo::Shader::CreateFromFile("./assets/shaders/sprite.vertex", "./assets/shaders/sprite.fragment");

		batch = new gueepo::SpriteBatcher();
		batch->Initialize(gueepo::Renderer::GetRendererAPI(), spriteShader);

		m_Camera = std::make_unique<gueepo::OrtographicCamera>(640, 360);
		m_Camera->SetBackgroundColor(0.33f, 0.0f, 0.33f, 1.0f);
	}

	void OnDetach() override {}
	void OnUpdate(float DeltaTime) override {}
	void OnInput(const gueepo::InputState& currentInputState) override {}
	void OnEvent(gueepo::Event& e) override {}
	void OnRender() override {
		gueepo::Color bgColor = m_Camera->GetBackGroundColor();
		gueepo::Renderer::Clear(bgColor.rgba[0], bgColor.rgba[1], bgColor.rgba[2], bgColor.rgba[3]);

		batch->Begin(*m_Camera);

		// batch->Draw(mainHero, mainHeroPosition.x, mainHeroPosition.y, 64, 64);

		batch->End();
	}
	void OnImGuiRender() override {}
private:
	std::unique_ptr<gueepo::OrtographicCamera> m_Camera;
};

class GBGJ1 : public gueepo::Application {
public:
	GBGJ1() : Application("gbgj-1", 640, 360) { PushLayer(new GameLayer()); }
	~GBGJ1() {}
};

gueepo::Application* gueepo::CreateApplication() {
	return new GBGJ1();
}
