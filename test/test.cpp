#include "interface_generator.h"

#include <gtest/gtest.h>

static const std::string root_dir = "./";

using namespace glasssix;

TEST(interface_generator, load_file)
{
    ymer::interface_generator generator;
    ASSERT_TRUE(generator.load_field_map(root_dir + "data/field.json"));
    ASSERT_TRUE(generator.load_predefined(root_dir + "data/predefined.hpp"));
    ASSERT_TRUE(generator.load_template(
        root_dir + "data/interface_template.hpp",
        root_dir + "data/impl_template.hpp",
        root_dir + "data/function_template.hpp"));
}



class generateTest :public testing::Test
{
protected:
    virtual void SetUp()
    {
        ASSERT_TRUE(generator_.load_field_map(root_dir + "data/field.json"));
        ASSERT_TRUE(generator_.load_predefined(root_dir + "data/predefined.hpp"));
        ASSERT_TRUE(generator_.load_template(
            root_dir + "data/interface_template.hpp",
            root_dir + "data/impl_template.hpp",
            root_dir + "data/function_template.hpp"));
        generator_.set_output_path("out_interface");
    }
    virtual void TearDown()
    {
    }

    ymer::interface_generator generator_;
};

TEST_F(generateTest, Object)
{
    ASSERT_TRUE(generator_.run(root_dir + "idl_file/Object.h"));
    // [read & write attributes] + [read only attributes] + [write only attributes] + [function]
    // 14 * 2 + 1 + 1 + 1 = 31
    ASSERT_EQ(generator_.function_signature().size(), 31);
}

TEST_F(generateTest, PixPoint)
{
    ASSERT_TRUE(generator_.run(root_dir + "idl_file/PixPoint.h"));
    ASSERT_EQ(generator_.function_signature().size(), 6);// 3 attributes (readable and writable)
}

TEST_F(generateTest, Image1)
{
    ASSERT_FALSE(generator_.run(root_dir + "idl_file/Image.h"));// Dependency (PixPoint) not found
    ASSERT_EQ(generator_.function_signature().size(), 0);
}

TEST_F(generateTest, Image2)
{
    generator_.set_include_path(std::vector<std::string>{ root_dir + "idl_file" });
    ASSERT_TRUE(generator_.run(root_dir + "idl_file/Image.h"));
    ASSERT_EQ(generator_.function_signature().size(), 6);// 1 attributes (readable) + 5 interface
}

TEST_F(generateTest, Widget)
{
    ASSERT_TRUE(generator_.run(root_dir + "idl_file/Widget.h"));
    ASSERT_EQ(generator_.function_signature().size(), 18);
}


int main(int argc, char** argv)
{
	printf("%s\n", argv[0]);
	printf("Running main() from %s\n", __FILE__);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
