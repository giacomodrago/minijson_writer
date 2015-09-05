#include "minijson_writer.hpp"

#include <sstream>
#include <stdexcept>

#include <gtest/gtest.h>

#define CPP11_SUPPORTED __cplusplus > 199711L || _MSC_VER >= 1800

#if CPP11_SUPPORTED
#define FINAL final
#else
#define FINAL
#endif

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

double return_zero() // to suppress VS2013 compiler errors
{
    return 0.0;
}

#pragma warning (push)
#pragma warning (disable: 4723)
TEST(minijson_writer, invalid_floats)
{
    std::stringstream stream;
    minijson::object_writer writer(stream);
    writer.write("posinfinity", 1.0 / return_zero());
    writer.write("neginfinity", -1.0 / return_zero());
    writer.write("nan", 0.0 / return_zero());
    writer.close();
    ASSERT_EQ("{\"posinfinity\":null,\"neginfinity\":null,\"nan\":null}", stream.str());
}
#pragma warning (pop)

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

struct point_type_writer FINAL
{
    void operator()(std::ostream& stream, point_type value) /* intentionally non-const */
    {
        const char* str = "";
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

void point_type_writer_func(std::ostream& stream, point_type value)
{
    point_type_writer()(stream, value);
}

namespace minijson
{

template<>
struct default_value_writer<point3d> FINAL
{
    void operator()(std::ostream& stream, const point3d& value, const writer_configuration& configuration) const
    {
        minijson::object_writer writer(stream, configuration);
        writer.write("x", value.x);
        writer.write("y", value.y);
        writer.write("z", value.z);
        writer.close();
    }
};

} // namespace minijson

void point3d_writer_func(std::ostream& stream, const point3d& value, minijson::writer_configuration configuration)
{
    minijson::default_value_writer<point3d>()(stream, value, configuration);
}

TEST(minijson_writer, custom_value_writer_object)
{
    std::stringstream stream;

    const point_type types[] = { FIXED, MOVING };

    const point3d point1 = { -1, 1, 0 };
    const point3d point2 = { 1, 1, 3 };
    std::vector<point3d> points;
    points.push_back(point1);
    points.push_back(point2);

    minijson::object_writer writer(stream);
    writer.write("type1", FIXED, point_type_writer()); // functor
    writer.write("type2", MOVING, point_type_writer_func); // function
    writer.write("point1", point1); // template specialisation
    writer.write("point2", point2, point3d_writer_func); // function
    writer.write_array("points", points.begin(), points.end()); // template specialisation
    writer.write_array("types1", types, types + 2, point_type_writer()); // functor
    writer.write_array("types2", types, types + 2, point_type_writer_func); // function
    writer.close();

    ASSERT_EQ("{\"type1\":\"fixed\","
               "\"type2\":\"moving\","
               "\"point1\":{\"x\":-1,\"y\":1,\"z\":0},"
               "\"point2\":{\"x\":1,\"y\":1,\"z\":3},"
               "\"points\":[{\"x\":-1,\"y\":1,\"z\":0},{\"x\":1,\"y\":1,\"z\":3}],"
               "\"types1\":[\"fixed\",\"moving\"],"
               "\"types2\":[\"fixed\",\"moving\"]}", stream.str());
}

TEST(minijson_writer, custom_value_writer_array)
{
    const point_type types[] = { FIXED, MOVING };

    {
        std::stringstream stream;

        const point_type types[] = { FIXED, MOVING };

        const point3d point1 = { -1, 1, 0 };
        const point3d point2 = { 1, 1, 3 };
        std::vector<point3d> points;
        points.push_back(point1);
        points.push_back(point2);

        minijson::array_writer writer(stream);
        writer.write(FIXED, point_type_writer()); // functor
        writer.write(MOVING, point_type_writer_func); // function
        writer.write(point1); // template specialisation
        writer.write(point2, point3d_writer_func); // function
        writer.write_array(points.begin(), points.end()); // template specialisation
        writer.write_array(types, types + 2, point_type_writer()); // functor
        writer.write_array(types, types + 2, point_type_writer_func); // function
        writer.close();

        ASSERT_EQ("[\"fixed\","
                   "\"moving\","
                   "{\"x\":-1,\"y\":1,\"z\":0},"
                   "{\"x\":1,\"y\":1,\"z\":3},"
                   "[{\"x\":-1,\"y\":1,\"z\":0},{\"x\":1,\"y\":1,\"z\":3}],"
                   "[\"fixed\",\"moving\"],"
                   "[\"fixed\",\"moving\"]]", stream.str());
    }

    {
        std::stringstream stream;

        minijson::write_array(stream, types, types + 2, point_type_writer()); // functor
        ASSERT_EQ("[\"fixed\",\"moving\"]", stream.str());
    }

    {
        const point3d point1 = { -1, 1, 0 };
        const point3d point2 = { 1, 1, 3 };
        std::vector<point3d> points;
        points.push_back(point1);
        points.push_back(point2);

        std::stringstream stream;

        minijson::write_array(stream, points.begin(), points.end()); // template specialisation
        ASSERT_EQ("[{\"x\":-1,\"y\":1,\"z\":0},{\"x\":1,\"y\":1,\"z\":3}]", stream.str());
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

TEST(minijson_writer, pretty_printing_nested)
{
    std::stringstream stream;

    minijson::array_writer writer(stream, minijson::writer_configuration().pretty_printing(true));
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

    const std::string expected =
        "[\n"
        "    \"value1\",\n"
        "    {\n"
        "        \"field2\": \"value2\",\n"
        "        \"nested2\": [\n"
        "            \"value3\",\n"
        "            \"value4\",\n"
        "            [\n"
        "                \"value5\",\n"
        "                {}\n"
        "            ],\n"
        "            \"value6\"\n"
        "        ],\n"
        "        \"nestedempty\": []\n"
        "    }\n"
        "]";

    ASSERT_EQ(expected, stream.str());
}

TEST(minijson_writer, pretty_printing_nested_functor)
{
    std::stringstream stream;

    std::vector<point3d> points(2);
    const point3d point0 = { -1, 1, 0 };
    const point3d point1 = { 2, 3, 4 };
    points[0] = point0;
    points[1] = point1;

    minijson::object_writer writer(stream, minijson::writer_configuration().pretty_printing(true).use_tabs(true));
    writer.write("point1", points[0]);
    writer.write_array("array", points.begin(), points.end());
    writer.close();

    const std::string expected =
        "{\n"
        "\t\"point1\": {\n"
        "\t\t\"x\": -1,\n"
        "\t\t\"y\": 1,\n"
        "\t\t\"z\": 0\n"
        "\t},\n"
        "\t\"array\": [\n"
        "\t\t{\n"
        "\t\t\t\"x\": -1,\n"
        "\t\t\t\"y\": 1,\n"
        "\t\t\t\"z\": 0\n"
        "\t\t},\n"
        "\t\t{\n"
        "\t\t\t\"x\": 2,\n"
        "\t\t\t\"y\": 3,\n"
        "\t\t\t\"z\": 4\n"
        "\t\t}\n"
        "\t]\n"
        "}";

    ASSERT_EQ(expected, stream.str());
}

TEST(minijson_writer, pretty_printing_write_array)
{
    std::vector<std::string> elements;
    elements.push_back("foo");
    elements.push_back("bar");

    std::stringstream stream;
    minijson::write_array(stream, elements.begin(), elements.end(),
            minijson::writer_configuration().pretty_printing(true).indent_spaces(2));

    const std::string expected =
        "[\n"
        "  \"foo\",\n"
        "  \"bar\"\n"
        "]";

    ASSERT_EQ(expected, stream.str());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
