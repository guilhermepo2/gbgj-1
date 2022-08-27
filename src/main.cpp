#include <gueepo2d.h>

class Sample : public gueepo::Application {
public:
	Sample() : Application("sample ", 640, 360) {}
	~Sample() {}
};

gueepo::Application* gueepo::CreateApplication() {
	return new Sample();
}
