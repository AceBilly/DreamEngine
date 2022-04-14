// #include <ReflectionMarco.h>

#define GENERATE_CLASS_BODY() __attribute__((annotate("reflection_class")))

class Base {
public:
	Base() = default;
	~Base() = default;
	int field;
	int field2 = 2;
private:
	int field3 = 0;
	int field4;
};


class GENERATE_CLASS_BODY() Derive : public Base {
public:
	int field5;
private:
	int field6;
};