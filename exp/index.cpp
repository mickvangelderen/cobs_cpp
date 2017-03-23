#include <iostream>
#include <random>
#include <cstdint>

template<typename A, typename B>
struct Variant {
private:
    enum class Type {
        AT,
        BT
    };

    union Data {
        A a;
        B b;
    };

    const Type type;
    const Data data;

public:
    Variant(A a) : type(Type::AT), data { .a = a } {};
    Variant(B b) : type(Type::BT), data { .b = b } {};

    // How do we write this to have map be able to determine the result?
    // https://gist.github.com/tibordp/6909880
    // http://stackoverflow.com/questions/5481539/what-does-t-double-ampersand-mean-in-c11
    template<typename Result, typename MapA, typename MapB>
    Result map(MapA map_a, MapB map_b) {
        switch (type) {
        case Type::AT: return map_a(data.a);
        case Type::BT: return map_b(data.b);
        }
    }
};

struct Robot {
    uint32_t wheel_count;
};

struct Cat {
    uint16_t leg_count;
    uint64_t eye_count;
};

typedef Variant<Robot, Cat> Result;

Result something() {
    std::random_device random_device;
    std::default_random_engine random_engine(random_device());
    std::uniform_int_distribution<int> zero_or_one(0, 1);
    int type = zero_or_one(random_engine);
    switch (type) {
    case 1: return Result { Robot { .wheel_count = 3 } };
    case 2: return Result { Cat { .leg_count = 4, .eye_count = 2 } };
    }
    std::cerr << "Never should have come here!" << std::endl;
    exit(EXIT_FAILURE);
}

int main() {
    std::cout << "Robot size: "<< sizeof(Robot) << std::endl;
    std::cout << "Cat size: "<< sizeof(Cat) << std::endl;
    std::cout << "Result size: "<< sizeof(Result) << std::endl;

    auto result = something();

    auto& out = std::cout;

    result.map<void>(
        [&out] (const Robot& robot) {
            out << "Wow its a robot! Beep boop I have " << robot.wheel_count << " wheels." << std::endl;
        },
        [&out] (const Cat& cat) {
            out << "Aww how adorable, it's a cat with " << cat.leg_count << " legs and " << cat.eye_count << " eyes." << std::endl;
        }
    );

    uint64_t total_count = result.map<uint64_t>(
        [] (const Robot& robot) { return robot.wheel_count; },
        [] (const Cat& cat) { return cat.leg_count + cat.eye_count; }
    );

    out << "At least the total number of things is " << total_count << std::endl;
}
