#include "minijson_writer.hpp"

#include <sstream>

#include <gtest/gtest.h>

#define CPP11_SUPPORTED __cplusplus > 199711L || _MSC_VER >= 1800

TEST(minijson_writer, empty_object)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.close();
    writer.close(); // double call to closed
    writer.write("foo", "bar"); // should be ignored
    ASSERT_EQ("{}", stream.str());
}

TEST(minijson_writer, empty_array)
{
    std::stringstream stream;
    minijson::array_writer writer(stream);
    writer.close();
    writer.close(); // double call to closed
    writer.write("bar"); // should be ignored
    ASSERT_EQ("[]", stream.str());
}

TEST(minijson_writer, single_element_object)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.write("int", 42);
    writer.close();
    writer.close();
    ASSERT_EQ("{\"int\":42}", stream.str());
}

TEST(minijson_writer, single_element_array)
{
    std::stringstream stream;
    minijson::array_writer writer(stream);
    writer.write("foo");
    writer.close();
    writer.close();
    ASSERT_EQ("[\"foo\"]", stream.str());
}

TEST(minijson_writer, basic_object)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.write("int", 42);
    writer.write("true", true);
    writer.write("false", false);
    writer.write("double", 42.42);
    writer.write("char*", "foo");
    writer.write("string", std::string("bar"));
    writer.write("null1", minijson::null);
#if CPP11_SUPPORTED
    writer.write("null2", nullptr);
#else
    writer.write("null2", minijson::null);
#endif
    writer.close();
    ASSERT_EQ("{\"int\":42,\"true\":true,\"false\":false,\"double\":42.42,\"char*\":\"foo\",\"string\":\"bar\",\"null1\":null,\"null2\":null}", stream.str());
}

TEST(minijson_writer, basic_array)
{
    std::stringstream stream;
    minijson::array_writer writer(stream);
    writer.write(42);
    writer.write(true);
    writer.write(false);
    writer.write(42.42);
    writer.write("foo");
    writer.write(std::string("bar"));
    writer.write(minijson::null);
#if CPP11_SUPPORTED
    writer.write(nullptr);
#else
    writer.write(minijson::null);
#endif
    writer.close();
    ASSERT_EQ("[42,true,false,42.42,\"foo\",\"bar\",null,null]", stream.str());
}

TEST(minijson_writer, escaping)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.write("\\\"\"\1\x1f\x7f\n\t\r", "a\"\\b");
    writer.write("int", 42); // make sure the stream flags are correctly restored after writing hex values
    writer.close();
    ASSERT_EQ("{\"\\\\\\\"\\\"\\u0001\\u001f\\u007f\\n\\t\\r\":\"a\\\"\\\\b\",\"int\":42}", stream.str());
}

TEST(minijson_writer, empty_string)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.write("", "");
    writer.close();
    ASSERT_EQ("{\"\":\"\"}", stream.str());
}

TEST(minijson_writer, nesting_simple)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    {
        minijson::object_writer nested_writer = writer.nested_object("nested");
        nested_writer.write("foo", "bar");
        nested_writer.close();
    }
    writer.close();
    ASSERT_EQ("{\"nested\":{\"foo\":\"bar\"}}", stream.str());
}

TEST(minijson_writer, nesting_complex)
{
    std::stringstream stream;

    minijson::array_writer writer(stream);
    writer.write("value1");
    {
        minijson::object_writer nested_writer1 = writer.nested_object();
        nested_writer1.write("field2", "value2");
        {
            minijson::array_writer nested_writer2 = nested_writer1.nested_array("nested2");
            nested_writer2.write("value3");
            nested_writer2.write("value4");
            {
                minijson::array_writer nested_writer3 = nested_writer2.nested_array();
                nested_writer3.write("value5");
                nested_writer3.nested_object().close();
                nested_writer3.close();
            }
            nested_writer2.write("value6");
            nested_writer2.close();
        }
        nested_writer1.nested_array("nestedempty").close();
        nested_writer1.close();
    }
    writer.close();

    ASSERT_EQ(
        "[\"value1\",{\"field2\":\"value2\",\"nested2\":[\"value3\",\"value4\",[\"value5\",{}],\"value6\"],\"nestedempty\":[]}]",
        stream.str());
}

TEST(minijson_writer, write_array)
{
    std::vector<std::string> elements;
    elements.push_back("nitrogen");
    elements.push_back("oxygen");

    {
        std::stringstream stream;
        minijson::object_writer writer(stream);
        writer.write_array("elements", elements.begin(), elements.end());
        writer.close();
        ASSERT_EQ("{\"elements\":[\"nitrogen\",\"oxygen\"]}", stream.str());
    }
    {
        std::stringstream stream;
        minijson::array_writer writer(stream);
        writer.write_array(elements.begin(), elements.end());
        writer.close();
        ASSERT_EQ("[[\"nitrogen\",\"oxygen\"]]", stream.str());
    }
    {
        std::stringstream stream;
        minijson::write_array(stream, elements.begin(), elements.end());
        ASSERT_EQ("[\"nitrogen\",\"oxygen\"]", stream.str());
    }
}

TEST(minijson_writer, utf8)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.write("à\"èẁ\"", "你\\好!");
    writer.close();
    ASSERT_EQ("{\"à\\\"èẁ\\\"\":\"你\\\\好!\"}", stream.str());
}

static double return_zero() // to suppress VS2013 compiler errors
{
    return 0.0;
}

TEST(minijson_writer, invalid_floats)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.write("posinfinity", 1.0 / return_zero());
    writer.write("neginfinity", -1.0 / return_zero());
    writer.write("nan", return_zero() / return_zero());
    writer.close();
    ASSERT_EQ("{\"posinfinity\":null,\"neginfinity\":null,\"nan\":null}", stream.str());
}

TEST(minijson_writer, float_formatting)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(12);
    minijson::array_writer writer(stream);
    writer.write(3.1415926535897);
    writer.close();
    ASSERT_EQ("[3.141592653590]", stream.str());
}

TEST(minijson_writer, bad_stream_flags)
{
    std::stringstream stream;
    minijson::array_writer writer(stream);
    stream << std::showpoint;
    writer.write(42.0);
    stream << std::showpos;
    writer.write(42.0);
    stream << std::hex << std::setw(10) << std::setfill('_');
    writer.write(42);
    writer.close();
    ASSERT_EQ("[42,42,42]", stream.str());
}

enum point_type
{
    FIXED, MOVING
};

struct point3d
{
    double x;
    double y;
    double z;
};

struct point_type_writer
{
    void operator()(std::ostream& stream, point_type value) const
    {
        const char* str;
        switch (value)
        {
        case FIXED:
            str = "fixed";
            break;
        case MOVING:
            str = "moving";
            break;
        }

        minijson::default_value_writer<char*>()(stream, str);
    }
};

namespace minijson
{

template<>
struct default_value_writer<point3d>
{
    void operator()(std::ostream& stream, const point3d& value) const
    {
        minijson::object_writer writer(stream);
        writer.write("x", value.x);
        writer.write("y", value.y);
        writer.write("z", value.z);
        writer.close();
    }
};

} // namespace minijson

TEST(minijson_writer, custom_value_writer_object)
{
    const point_type types[] = { FIXED, MOVING };

    std::stringstream stream;

    const point_type type = MOVING;
    const point3d point = { -1, 1, 0 };

    minijson::object_writer writer(stream);
    writer.write("type", type, point_type_writer()); // using functor
    writer.write("point", point); // using template specialisation
    writer.write_array("types", types, types + 2, point_type_writer()); // write_array with functor
    writer.close();

    ASSERT_EQ("{\"type\":\"moving\",\"point\":{\"x\":-1,\"y\":1,\"z\":0},\"types\":[\"fixed\",\"moving\"]}", stream.str());
}

TEST(minijson_writer, custom_value_writer_array)
{
    const point_type types[] = { FIXED, MOVING };

    {
        std::stringstream stream;

        const point_type type = MOVING;
        const point3d point = { -1, 1, 0 };

        minijson::array_writer writer(stream);
        writer.write(type, point_type_writer()); // using functor
        writer.write(point); // using template specialisation
        writer.write_array(types, types + 2, point_type_writer()); // write_array with functor
        writer.close();

        ASSERT_EQ("[\"moving\",{\"x\":-1,\"y\":1,\"z\":0},[\"fixed\",\"moving\"]]", stream.str());
    }

    {
        std::stringstream stream;

        minijson::write_array(stream, types, types + 2, point_type_writer());
        ASSERT_EQ("[\"fixed\",\"moving\"]", stream.str());
    }
}

TEST(minijson_writer, remove_locale)
{
    std::stringstream stream;

    try
    {
        stream.imbue(std::locale("en_US.utf8"));
    }
    catch (const std::runtime_error&)
    {
        // The locale is not supported: we can't run this test.
        std::cout << "Locale not supported: cannot run test" << std::endl;
        return;
    }

    stream << 1000.25;
    ASSERT_EQ("1,000.25", stream.str());

    stream.str("");

    minijson::object_writer writer(stream);
    writer.write("foo", 1000.25);
    writer.close();

    ASSERT_EQ("{\"foo\":1000.25}", stream.str());
}

TEST(minijson_writer, long_strings)
{
    std::ostringstream stream;
    minijson::object_writer writer(stream);

    writer.write("field0", "Lorem ipsum dolor sit amet");
    writer.write("field1", true);
    writer.write("field2", "f");
    writer.write("field3", std::string("Quisque finibus sodales turpis eu commodo. "
                                       "Cras scelerisque dignissim turpis,\nsed ullamcorper felis rutrum sed. "
                                       "Vestibulum dictum elit turpis,\3 nec laoreet est rutrum ut."));
    writer.write("field4", minijson::null);
    writer.write("field5", 42);
    writer.write("field6", "Aenean est mi, facilisis auctor tincidunt nec, ullamcorper ac ipsum. "
                           "Aliquam metus quam, auctor nec tortor in, scelerisque porta nulla. "
                           "Nulla facilisis scelerisque ipsum, in sagittis lorem scelerisque at. "
                           "Morbi tincidunt orci vel porttitor fringilla. Nullam tristique justo ut ultricies tincidunt. "
                           "Phasellus eu magna dolor. Mauris ut aliquet velit. Nullam id faucibus justo. "
                           "Fusce ultricies blandit lacinia. Nulla auctor augue mi, sit amet consectetur ante imperdiet ac.");
    writer.write("field7", 42.42);
    writer.close();

    ASSERT_EQ(777U, stream.str().size());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
