# minijson_writer

## Motivation and design

When writing JSON messages, some C/C++ libraries work by building an in-memory object for the sole purpose of serialising it as JSON. Many libraries comprise multiple header and source files and/or expose a complex API.

`minijson_writer` is a simple decorator around a `std::ostream`. It directly writes on the stream without allocating additional memory, and does not throw exceptions unless the stream does.

Despite being a single header file of ~500 lines, `minijson_writer` is complete and can be easily extended to support custom types.

`minijson_reader` (currently under development) will be the independent counterpart for parsing JSON messages.

## Dependencies

If the compiler does **not** support C++11, Boost 1.56 or better is required.

## Basic usage

Writing a JSON **object**:

```
minijson::object_writer writer(stream); // wrap a std::ostream
writer.write("field1", 42); // integral types
writer.write("field2", true); // boolean
writer.write("field3", 42.42); // floating point types
writer.write("field4", "foo"); // char[] and char*
writer.write("field5", std::string("bar")); // std::string
writer.write("field6", minijson::null); // null type (on C++11 you can use nullptr)
writer.close(); // always call close() when you are done
```

Writing a JSON **array**:

```
minijson::array_writer writer(stream); // wrap a std::ostream
writer.write(42); // integral types
writer.write(true); // boolean
writer.write(42.42); // floating point types
writer.write("foo"); // char[] and char*
writer.write(std::string("bar")); // std::string
writer.write(minijson::null); // null type (on C++11 you can use nullptr)
writer.close(); // always call close() when you are done
```

## Nested objects and arrays

Both `object_writer` and` array_writer` have two methods called `nested_object()` and `nested_array()` returning another writer that can be used to write a nested object or a nested array, respectively.

```
minijson::object_writer writer(stream);
writer.write("name", "Los Angeles");
{
  minijson::object_writer position_writer = writer.nested_object("position");
  position_writer.write("n", 34.05);
  position_writer.write("w", 118.25);
  position_writer.close();
}
{
  minijson::array_writer mayors_writer = writer.nested_array("mayors");
  mayors_writer.write("Villaraigosa");
  mayors_writer.write("Garcetti");
  mayors_writer.close();
}
writer.close();
```

Arrays can be also written by using the `write_array` method, that accepts a range:

```
const char* mayors[] = { "Villaraigosa", "Garcetti" };
minijson::object_writer writer(stream);
writer.write("name", "Los Angeles");
writer.write_array("mayors", std::begin(mayors), std::end(mayors));
writer.close();
```

A utility `write_array` function is provided, that can be used to write top-level JSON arrays:

```
const char* mayors[] = { "Villaraigosa", "Garcetti" };
minijson::write_array(stream, std::begin(mayors), std::end(mayors));
```

## Extensions

As a (possibly) neater alternative to `nested_object()` and `nested_array()`, you can provide support for custom types by specialising `minijson::default_value_writer`:

```
struct position
{
  double n;
  double w;
};

namespace minijson
{

template<>
struct default_value_writer<position>
{
  void operator()(std::ostream& stream, const position& p) const
  {
    minijson::object_writer writer(stream);
    writer.write("n", p.n);
    writer.write("w", p.w);
    writer.close();
  }
};

} // namespace minijson
```

You can now write `position` instances right away:

```
const position p = { 34.05, 118.25 };

minijson::object_writer writer(stream);
writer.write("name", "Los Angeles");
writer.write("position", p);
writer.close();
```

You are encouraged to specialise `default_value_writer` for your custom types, but please note that specialising it for primitive types or types declared in the `std` namespace could break your future builds, in case contributors to this library decide to provide more specialisations.

As an alternative to specialising `default_value_writer`, you can provide a functor when you call `write`:

```
enum party
{
  REPUBLICANS,
  DEMOCRATS,
  OTHER
};

struct party_writer
{
  void operator()(std::ostream& stream, const party& p) const
  {
    const char* s;
    switch (p)
    {
    case REPUBLICANS: s = "republicans"; break;
    case DEMOCRATS:   s = "democrats";   break;
    default:          s = "other";       break;
    }
    minijson::default_value_writer<char*>()(stream, s); // always write quoted strings this way
  }
};

// ...

minijson::object_writer writer(stream);
writer.write("name", "Los Angeles");
writer.write("governedBy", DEMOCRATS, party_writer());
writer.close();
```

Similarly, a functor can be provided to `write_array` (both the method and the standalone function) to determine how each item of the range has to be written.

## Remarks

### Encodings

**ASCII** and **UTF-8** strings are supported. The output encoding depends on the encoding of the input strings (field names and values), and no transformations are performed besides escaping control characters.

### Pretty-printing

Pretty-printing is currently not supported.

### The stream

- You can use `std::fixed`, `std::scientific`, and `std::setprecision` on the stream to modify how **floating-point numbers** are represented
- All the other format flags may be altered by this library, and never restored to their original value
- The stream is never flushed, not even when `close()` is called
- It is responsibility of the client to check the stream's *error state* flags

### Exceptions

In general, no exceptions are thrown unless the stream does: in that case, [basic exception safety](http://en.wikipedia.org/wiki/Exception_safety) is guaranteed. Constructors and destructors do not affect the stream, thus being no-throw.

### Copy construction

Copying a writer is allowed (for C++03 compatibility, e.g. when storing writers into STL containers) and is safe, as long as only **one** copy is then used for writing on the stream.