from ctypes import *
import capnp
libcommkit = CDLL('libcommkit.dylib')

class COpts(Structure):
    _fields_ = [
        ("reliable", c_bool),
    ]
    
class CPayload(Structure):
    _fields_ = [
        ("bytes", POINTER(c_ubyte)),
        ("len", c_size_t),
        ("sequence", c_uint64),
        ("source_timestamp", c_uint64),
    ]
    
    def to_python(self, topic):
        return Payload(topic.decode_msg(string_at(self.bytes, self.len)), self.sequence, self.source_timestamp)

class Topic:
    def __init__(self, name, datatype, reliable=False):
        self.name = name
        self.datatype = datatype
        self.reliable = reliable
        
    def is_capnp(self):
        return isinstance(self.datatype, capnp._StructModule)
        
    def datatype_name(self):
        if self.is_capnp():
            return  "{:016x}".format(self.datatype.schema.node.id)
        else:
            return self.datatype
        
    def decode_msg(self, bytes):
        if self.is_capnp():
            return self.datatype.from_bytes(bytes)
        else:
            return bytes
        
    def encode_msg(self, msg):
        if self.is_capnp():
            return bytearray(msg.to_bytes())
        else:
            return bytearray(msg)

class Node:
    def __init__(self, name):
        self._as_parameter_ = libcommkit.commkit_node_create(name)
        
    def from_param(self): return self._as_parameter_
        
    def __del__(self):
        libcommkit.commkit_node_destroy(self)
        
    def subscribe(self, topic):
        opts = COpts()
        opts.reliable = topic.reliable
        return Subscriber(libcommkit.commkit_subscriber_create(self, topic.name, topic.datatype_name(), byref(opts)), topic)
        
    def publish(self, topic):
        opts = COpts()
        opts.reliable = topic.reliable
        return Publisher(libcommkit.commkit_publisher_create(self, topic.name, topic.datatype_name(), byref(opts)), topic)
        
class Subscriber:
    def __init__(self, obj, topic):
        self._as_parameter_ = obj
        self.topic = topic
        
    def from_param(self): return self._as_parameter_
        
    def __del__(self):
        libcommkit.commkit_subscriber_destroy(self)
        
    def wait_for_message(self):
        libcommkit.commkit_wait_for_message(self)
        
    def matched_publishers(self):
        return libcommkit.commkit_matched_publishers(self)
        
    def peek(self):
        p = CPayload()
        if libcommkit.commkit_peek(self, byref(p)):
            return p.to_python(self.topic)
        else:
            return None
        
    def take(self):
        p = CPayload()
        if libcommkit.commkit_take(self, byref(p)):
            return p.to_python(self.topic)
        else:
            return None
        
class Payload:
    def __init__(self, data, sequence, source_timestamp):
        self.data = data
        self.sequence = sequence
        self.source_timestamp = source_timestamp

class Publisher:
    def __init__(self, obj, topic):
        self._as_parameter_ = obj
        self.topic = topic
        
    def from_param(self): return self._as_parameter_
        
    def __del__(self):
        libcommkit.commkit_publisher_destroy(self)
        
    def publish(self, data):
        data = self.topic.encode_msg(data)
        return libcommkit.commkit_publish(self, byref(c_ubyte.from_buffer(data)), len(data))
    
    def matched_subscribers(self):
        return libcommkit.commkit_matched_subscribers(self)


def fntype(fn, res, args):
    f = getattr(libcommkit, fn)
    f.argtypes = args
    f.restype = res

fntype('commkit_node_create', c_void_p, (c_char_p,))    
fntype('commkit_node_destroy', None, (c_void_p,))

fntype('commkit_subscriber_create', c_void_p, (c_void_p, c_char_p, c_char_p, POINTER(COpts)))
fntype('commkit_subscriber_destroy', None, (c_void_p,))

fntype('commkit_publisher_create', c_void_p, (c_void_p, c_char_p, c_char_p, POINTER(COpts)))
fntype('commkit_publisher_destroy', None, (c_void_p,))

fntype('commkit_wait_for_message', None, (c_void_p,))
fntype('commkit_matched_publishers', None, (c_void_p,))
fntype('commkit_peek', c_bool, (c_void_p, POINTER(CPayload)))
fntype('commkit_take', c_bool, (c_void_p, POINTER(CPayload)))

fntype('commkit_publish', None, (c_void_p, POINTER(c_ubyte), c_size_t))
fntype('commkit_matched_subscribers', None, (c_void_p,))
