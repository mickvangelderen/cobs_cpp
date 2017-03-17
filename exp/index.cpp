#include <iostream>
#include <random>
#include <cstdint>

struct Result {
    // Types
public:
    struct Amoeba {};

    struct Robot {
        uint32_t wheel_count;
    };

    struct Cat {
        uint16_t leg_count;
        uint64_t eye_count;
    };

private:
    enum class Type {
        AMOEBA,
        ROBOT,
        CAT
    };

    union Data {
        Amoeba amoeba;
        Robot robot;
        Cat cat;
    };

    // Fields
    const Type type;
    const Data data;

    // Functions
public:
    Result(Amoeba amoeba) : type(Type::AMOEBA), data { .amoeba = amoeba } {};

    Result(Robot robot) : type(Type::ROBOT), data { .robot = robot } {};

    Result(Cat cat) : type(Type::CAT), data { .cat = cat } {};

    const Amoeba * amoeba() {
        return type == Type::AMOEBA ? &data.amoeba : nullptr;
    }

    const Robot * robot() {
        return type == Type::ROBOT ? &data.robot : nullptr;
    }

    const Cat * cat() {
        return type == Type::CAT ? &data.cat : nullptr;
    }
};

Result something() {
    std::random_device random_device;
    std::default_random_engine random_engine(random_device());
    std::uniform_int_distribution<int> zero_one_or_two(0, 2);
    int type = zero_one_or_two(random_engine);
    switch (type) {
    case 0: return Result { Result::Amoeba {} };
    case 1: return Result { Result::Robot { .wheel_count = 3 } };
    case 2: return Result { Result::Cat { .leg_count = 4, .eye_count = 2 } };
    }
    exit(-1);
}

int main() {
    auto result = something();

    std::cout << "Amoeba size: "<< sizeof(Result::Amoeba) << std::endl;
    std::cout << "Robot size: "<< sizeof(Result::Robot) << std::endl;
    std::cout << "Cat size: "<< sizeof(Result::Cat) << std::endl;
    std::cout << "Result size: "<< sizeof(Result) << std::endl;

    if (result.amoeba()) {
        std::cout << "Found an amoeba! It has no properties of interest to us." << std::endl;
    } else if (auto robot = result.robot()) {
        std::cout << "Wow its a robot! Beep boop I have " << robot->wheel_count << " wheels." << std::endl;
    } else if (auto cat = result.cat()) {
        std::cout << "Aww how adorable, it's a cat with " << cat->leg_count << " legs and " << cat->eye_count << " eyes." << std::endl;
    } else {
        std::cerr << "I have no idea what this is!" << std::endl;
        exit(-1);
    }
}
