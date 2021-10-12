#include "interface_generator.h"

#include <gtest/gtest.h>

static const std::string root_dir = "./";

using namespace glasssix;


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
		ASSERT_FALSE(generator_.set_output_path("out_interface").empty());
	}
	virtual void TearDown()
	{
	}

	ymer::interface_generator generator_;
};


/******************************************************
* Test interface code
*******************************************************/
TEST_F(generateTest, code1)
{
	ASSERT_FALSE(generator_.run_code(""));
	ASSERT_FALSE(generator_.run_code("    "));
	ASSERT_FALSE(generator_.run_code("xxx...xxx"));
}

TEST_F(generateTest, code2)
{
	ASSERT_TRUE(generator_.run_code(R"(
                namespace g6
                {
	                interface TestCode2
	                {
		                int32 val;
                         bool foo(float x) const;
	                };
                }
			)"));
	ASSERT_EQ(generator_.function_signature().size(), 3);

	ASSERT_FALSE(generator_.run_code("xxx"));
	ASSERT_TRUE(generator_.function_signature().empty());
}

TEST_F(generateTest, code3)
{
	ASSERT_FALSE(generator_.run_code(R"(
                namespace g6
                {
	                interface TestCode3
	                {
	                };
                }
			)"));
	ASSERT_TRUE(generator_.function_signature().empty());
}


TEST_F(generateTest, functionas)
{
	ASSERT_TRUE(generator_.function_signature().empty());
}



/******************************************************
* Test interface file
*******************************************************/
TEST_F(generateTest, Empty)
{
	ASSERT_FALSE(generator_.run_file(""));
	ASSERT_FALSE(generator_.run_file("file.abc"));
}

TEST_F(generateTest, Object)
{
	ASSERT_TRUE(generator_.run_file(root_dir + "idl_file/Object.h"));
	// [read & write attributes] + [read only attributes] + [write only attributes] + [function]
	// 14 * 2 + 1 + 1 + 1 = 31
	ASSERT_EQ(generator_.function_signature().size(), 31);
}

TEST_F(generateTest, PixPoint)
{
	ASSERT_TRUE(generator_.run_file(root_dir + "idl_file/PixPoint.h"));
	ASSERT_EQ(generator_.function_signature().size(), 6);// 3 attributes (readable and writable)
}

TEST_F(generateTest, Image1)
{
	ASSERT_FALSE(generator_.run_file(root_dir + "idl_file/Image.h"));// Dependency (PixPoint) not found
	ASSERT_EQ(generator_.function_signature().size(), 0);
}

TEST_F(generateTest, Image2)
{
	generator_.set_include_path(std::vector<std::string>{ root_dir + "idl_file" });
	ASSERT_TRUE(generator_.run_file(root_dir + "idl_file/Image.h"));
	ASSERT_EQ(generator_.function_signature().size(), 6);// 1 attributes (readable) + 5 interface
}

TEST_F(generateTest, Widget)
{
	ASSERT_TRUE(generator_.run_file(root_dir + "idl_file/Widget.h"));
	ASSERT_EQ(generator_.function_signature().size(), 18);
}


TEST_F(generateTest, OutputPath)
{
	ASSERT_FALSE(generator_.set_output_path("").empty());// Convert to current path
	ASSERT_TRUE(generator_.run_code(R"(
                namespace g6 {
	                interface TestOutputPath {
                         bool foo(float x) const;
	                };
                }
			)"));
	ASSERT_EQ(generator_.function_signature().size(), 1);
}

TEST(interface_generator, OutputPathNotSet)
{
	// Default current path
	ymer::interface_generator generator;
	ASSERT_TRUE(generator.load_field_map(root_dir + "data/field.json"));
	ASSERT_TRUE(generator.load_predefined(root_dir + "data/predefined.hpp"));
	ASSERT_TRUE(generator.load_template(
		root_dir + "data/interface_template.hpp",
		root_dir + "data/impl_template.hpp",
		root_dir + "data/function_template.hpp"));
	ASSERT_TRUE(generator.run_code(R"(
                namespace g6 {
	                interface OutputPathNotSet {
                         bool foo(float x) const;
	                };
                }
			)"));
	ASSERT_EQ(generator.function_signature().size(), 1);
}



int main(int argc, char** argv)
{
	printf("%s\n", argv[0]);
	printf("run_filening main() from %s\n", __FILE__);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
