#ifndef __SDR_OPTIONS_HH__
#define __SDR_OPTIONS_HH__

#include <string>
#include <map>

class Options
{
public:
  class Value {
  protected:
    typedef enum {
      NONE, INTEGER, FLOAT, STRING
    } Type;

  public:
    Value();
    Value(long value);
    Value(double value);
    Value(const std::string &value);
    Value(const Value &other);
    ~Value();

    const Value &operator=(const Value &other);

    bool isNone() const;
    bool isInteger() const;
    bool isFloat() const;
    bool isString() const;

    long toInteger() const;
    double toFloat() const;
    std::string toString() const;

  protected:
    Type _type;
    union {
      long as_int;
      double as_float;
      char *as_string;
    } _value;
  };

  typedef enum {
    FLAG, INTEGER, FLOAT, ANY
  } ArgType;

  typedef struct {
    const char *name;
    char short_name;
    ArgType type;
    const char *help;
  } Definition;

  static bool parse(const Definition defs[], int argc, char *argv[], Options &options);

  static void print_help(std::ostream &stream, const Definition defs[]);

public:
  Options();
  bool has(const char *name);
  const Value &get(const char *name);

protected:
  std::map<std::string, Value> _options;
};

#endif // __SDR_QOPTIONS_HH__
