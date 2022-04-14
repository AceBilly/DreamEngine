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

class Derive : public Base {
public:
};