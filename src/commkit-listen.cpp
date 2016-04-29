#include <unistd.h>
#include <commkit/commkit.h>
#include <iostream>
#include <capnp/schema.h>
#include <capnp/dynamic.h>
#include <capnp/schema-loader.h>
#include <capnp/schema-parser.h>

static void usage()
{

    printf("usage: commkit-listen -s SCHEMA -t TYPE [-r] TOPIC\n");
    printf("    -s SCHEMA                              path to Cap'n Proto schema\n");
    printf("    -t TYPE                                struct name within the schema\n");
    printf("    -r                                     the topic uses RTPS 'reliable' QoS\n");
    printf("    TOPIC                                  RTPS topic on which to listen\n");
    printf("    -h                                     print help and exit\n");

    std::exit(1);
}

using capnp::DynamicValue;
using capnp::Text;
using capnp::DynamicList;
using capnp::DynamicEnum;
using capnp::DynamicStruct;
void dynamicPrintValue(DynamicValue::Reader value);

int main(int argc, char *argv[])
{
    int c;

    bool reliable = false;
    std::string topic_type;
    std::string schema_file;

    while ((c = getopt(argc, argv, "s:t:rh")) != -1) {
        switch (c) {
        case 'r':
            reliable = true;
            break;

        case 't':
            topic_type = std::string(optarg);
            break;

        case 's':
            schema_file = std::string(optarg);
            break;

        default:
            usage();
            break;
        }
    }

    if (optind + 1 > argc) {
        usage();
    }
    std::string topic_name(argv[optind]);

    kj::StringPtr importPath[] = {"/usr/include", "/usr/local/include", "/opt/include"};

    capnp::SchemaParser sp;
    capnp::ParsedSchema schema_root;
    capnp::StructSchema schema;
    std::string type_id;
    try {
        schema_root = sp.parseDiskFile(schema_file, schema_file, importPath);
        schema = schema_root.getNested(topic_type).asStruct();
        type_id = commkit::Topic::capn_type_id(schema);
    } catch (kj::Exception e) {
        std::cout << "Error loading schema: " << e.getDescription().cStr() << std::endl;
        exit(1);
    }

    std::cout << "Topic type " << topic_type << " with id " << type_id << std::endl;

    commkit::Node node;
    if (!node.init("commkit-listen")) {
        printf("error initializing node\n");
        exit(1);
    }

    commkit::Topic topic(topic_name, type_id, 1024);

    auto sub = node.createSubscriber(topic);
    if (sub == nullptr) {
        printf("error creating topic\n");
        exit(1);
    }

    commkit::SubscriptionOpts sub_opts;
    sub_opts.reliable = reliable;
    sub_opts.history = 1;
    if (!sub->init(sub_opts)) {
        printf("error subscribing\n");
        exit(1);
    }

    while (1) {
        sub->waitForMessage();
        commkit::Payload p;
        if (sub->take(&p)) {
            if (p.len % sizeof(capnp::word) == 0) {
                auto wb = kj::ArrayPtr<const capnp::word>(
                    reinterpret_cast<const capnp::word *>(p.bytes), p.len / sizeof(capnp::word));
                auto reader =
                    capnp::FlatArrayMessageReader(wb).getRoot<capnp::DynamicStruct>(schema);
                dynamicPrintValue(reader);
                std::cout << std::endl;
            } else {
                std::cout << "Invalid message length" << std::endl;
            }
        }
    }
}

void dynamicPrintValue(DynamicValue::Reader value)
{
    // From https://capnproto.org/cxx.html#dynamic-reflection
    // Print an arbitrary message via the dynamic API by
    // iterating over the schema.

    switch (value.getType()) {
    case DynamicValue::VOID:
        std::cout << "";
        break;
    case DynamicValue::BOOL:
        std::cout << (value.as<bool>() ? "true" : "false");
        break;
    case DynamicValue::INT:
        std::cout << value.as<int64_t>();
        break;
    case DynamicValue::UINT:
        std::cout << value.as<uint64_t>();
        break;
    case DynamicValue::FLOAT:
        std::cout << value.as<double>();
        break;
    case DynamicValue::TEXT:
        std::cout << '\"' << value.as<Text>().cStr() << '\"';
        break;
    case DynamicValue::LIST: {
        std::cout << "[";
        bool first = true;
        for (auto element : value.as<DynamicList>()) {
            if (first) {
                first = false;
            } else {
                std::cout << ", ";
            }
            dynamicPrintValue(element);
        }
        std::cout << "]";
        break;
    }
    case DynamicValue::ENUM: {
        auto enumValue = value.as<DynamicEnum>();
        KJ_IF_MAYBE(enumerant, enumValue.getEnumerant())
        {
            std::cout << enumerant->getProto().getName().cStr();
        }
        else
        {
            // Unknown enum value; output raw number.
            std::cout << enumValue.getRaw();
        }
        break;
    }
    case DynamicValue::STRUCT: {
        std::cout << "(";
        auto structValue = value.as<DynamicStruct>();
        bool first = true;
        for (auto field : structValue.getSchema().getFields()) {
            if (!structValue.has(field))
                continue;
            if (first) {
                first = false;
            } else {
                std::cout << ", ";
            }
            std::cout << field.getProto().getName().cStr() << " = ";
            dynamicPrintValue(structValue.get(field));
        }
        std::cout << ")";
        break;
    }
    default:
        // There are other types, we aren't handling them.
        std::cout << "?";
        break;
    }
}
