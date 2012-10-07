# This file was automatically generated by SWIG (http://www.swig.org).
# Version 2.0.4
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.



from sys import version_info
if version_info >= (2,6,0):
    def swig_import_helper():
        from os.path import dirname
        import imp
        fp = None
        try:
            fp, pathname, description = imp.find_module('_cqpid', [dirname(__file__)])
        except ImportError:
            import _cqpid
            return _cqpid
        if fp is not None:
            try:
                _mod = imp.load_module('_cqpid', fp, pathname, description)
            finally:
                fp.close()
            return _mod
    _cqpid = swig_import_helper()
    del swig_import_helper
else:
    import _cqpid
del version_info
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'SwigPyObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError(name)

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

try:
    _object = object
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


MessagingError = _cqpid.MessagingError

LinkError = _cqpid.LinkError

AddressError = _cqpid.AddressError

ResolutionError = _cqpid.ResolutionError

AssertionFailed = _cqpid.AssertionFailed

NotFound = _cqpid.NotFound

InvalidOption = _cqpid.InvalidOption

MalformedAddress = _cqpid.MalformedAddress

ReceiverError = _cqpid.ReceiverError

FetchError = _cqpid.FetchError

Empty = _cqpid.Empty

SenderError = _cqpid.SenderError

SendError = _cqpid.SendError

TargetCapacityExceeded = _cqpid.TargetCapacityExceeded

ConnectionError = _cqpid.ConnectionError

ConnectError = _cqpid.ConnectError

SessionError = _cqpid.SessionError

TransactionError = _cqpid.TransactionError

TransactionAborted = _cqpid.TransactionAborted

UnauthorizedAccess = _cqpid.UnauthorizedAccess

class Address(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Address, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Address, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_Address(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_Address
    __del__ = lambda self : None;
    def getName(self): return _cqpid.Address_getName(self)
    def setName(self, *args): return _cqpid.Address_setName(self, *args)
    def getSubject(self): return _cqpid.Address_getSubject(self)
    def setSubject(self, *args): return _cqpid.Address_setSubject(self, *args)
    def getOptions(self, *args): return _cqpid.Address_getOptions(self, *args)
    def setOptions(self, *args): return _cqpid.Address_setOptions(self, *args)
    def getType(self): return _cqpid.Address_getType(self)
    def setType(self, *args): return _cqpid.Address_setType(self, *args)
    def str(self): return _cqpid.Address_str(self)
    def __nonzero__(self):
        return _cqpid.Address___nonzero__(self)
    __bool__ = __nonzero__


Address_swigregister = _cqpid.Address_swigregister
Address_swigregister(Address)

class Duration(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Duration, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Duration, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_Duration(*args)
        try: self.this.append(this)
        except: self.this = this
    def getMilliseconds(self): return _cqpid.Duration_getMilliseconds(self)
    __swig_destroy__ = _cqpid.delete_Duration
    __del__ = lambda self : None;
Duration_swigregister = _cqpid.Duration_swigregister
Duration_swigregister(Duration)
cvar = _cqpid.cvar
Duration.FOREVER = _cqpid.cvar.Duration_FOREVER
Duration.IMMEDIATE = _cqpid.cvar.Duration_IMMEDIATE
Duration.SECOND = _cqpid.cvar.Duration_SECOND
Duration.MINUTE = _cqpid.cvar.Duration_MINUTE


def __eq__(*args):
  return _cqpid.__eq__(*args)
__eq__ = _cqpid.__eq__

def __ne__(*args):
  return _cqpid.__ne__(*args)
__ne__ = _cqpid.__ne__
class Message(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Message, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Message, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_Message(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_Message
    __del__ = lambda self : None;
    def _setReplyTo(self, *args): return _cqpid.Message__setReplyTo(self, *args)
    def _getReplyTo(self): return _cqpid.Message__getReplyTo(self)
    def setSubject(self, *args): return _cqpid.Message_setSubject(self, *args)
    def getSubject(self): return _cqpid.Message_getSubject(self)
    def setContentType(self, *args): return _cqpid.Message_setContentType(self, *args)
    def getContentType(self): return _cqpid.Message_getContentType(self)
    def setMessageId(self, *args): return _cqpid.Message_setMessageId(self, *args)
    def getMessageId(self): return _cqpid.Message_getMessageId(self)
    def setUserId(self, *args): return _cqpid.Message_setUserId(self, *args)
    def getUserId(self): return _cqpid.Message_getUserId(self)
    def setCorrelationId(self, *args): return _cqpid.Message_setCorrelationId(self, *args)
    def getCorrelationId(self): return _cqpid.Message_getCorrelationId(self)
    def setPriority(self, *args): return _cqpid.Message_setPriority(self, *args)
    def getPriority(self): return _cqpid.Message_getPriority(self)
    def _setTtl(self, *args): return _cqpid.Message__setTtl(self, *args)
    def _getTtl(self): return _cqpid.Message__getTtl(self)
    def setDurable(self, *args): return _cqpid.Message_setDurable(self, *args)
    def getDurable(self): return _cqpid.Message_getDurable(self)
    def getRedelivered(self): return _cqpid.Message_getRedelivered(self)
    def setRedelivered(self, *args): return _cqpid.Message_setRedelivered(self, *args)
    def getProperties(self, *args): return _cqpid.Message_getProperties(self, *args)
    def setContent(self, *args): return _cqpid.Message_setContent(self, *args)
    def getContent(self): return _cqpid.Message_getContent(self)
    def getContentPtr(self): return _cqpid.Message_getContentPtr(self)
    def getContentSize(self): return _cqpid.Message_getContentSize(self)
    def setProperty(self, *args): return _cqpid.Message_setProperty(self, *args)
    # UNSPECIFIED was module level before, but I do not
    # know how to insert python code at the top of the module.
    # (A bare "%pythoncode" inserts at the end.
    UNSPECIFIED=object()
    def __init__(self, content=None, content_type=UNSPECIFIED, id=None,
                 subject=None, user_id=None, reply_to=None,
                 correlation_id=None, durable=None, priority=None,
                 ttl=None, properties=None):
        this = _cqpid.new_Message('')
        try: self.this.append(this)
        except: self.this = this
        if content :
            self.content = content
        if content_type != UNSPECIFIED :
            self.content_type = content_type
        if id is not None :
            self.id = id
        if subject is not None :
            self.subject = subject
        if user_id is not None :
            self.user_id = user_id
        if reply_to is not None :
            self.reply_to = reply_to
        if correlation_id is not None :
            self.correlation_id = correlation_id
        if durable is not None :
            self.durable = durable
        if priority is not None :
            self.priority = priority
        if ttl is not None :
            self.ttl = ttl
        if properties is not None :
            # Can't set properties via (inst).getProperties, because
            # the typemaps make a copy of the underlying properties.
            # Instead, set via setProperty for the time-being
            for k, v in properties.iteritems() :
                self.setProperty(k, v)

    def _get_content(self) :
        if self.content_type == "amqp/list" :
            return decodeList(self)
        if self.content_type == "amqp/map" :
            return decodeMap(self)
        return self.getContent()
    def _set_content(self, content) :
        if isinstance(content, basestring) :
            self.setContent(content)
        elif isinstance(content, list) or isinstance(content, dict) :
            encode(content, self)
        else :
            # Not a type we can handle.  Try setting it anyway,
            # although this will probably lead to a swig error
            self.setContent(content)
    __swig_getmethods__["content"] = _get_content
    __swig_setmethods__["content"] = _set_content
    if _newclass: content = property(_get_content, _set_content)

    __swig_getmethods__["content_type"] = getContentType
    __swig_setmethods__["content_type"] = setContentType
    if _newclass: content_type = property(getContentType, setContentType)

    __swig_getmethods__["id"] = getMessageId
    __swig_setmethods__["id"] = setMessageId
    if _newclass: id = property(getMessageId, setMessageId)

    __swig_getmethods__["subject"] = getSubject
    __swig_setmethods__["subject"] = setSubject
    if _newclass: subject = property(getSubject, setSubject)

    __swig_getmethods__["priority"] = getPriority
    __swig_setmethods__["priority"] = setPriority
    if _newclass: priority = property(getPriority, setPriority)

    def getTtl(self) :
        return self._getTtl().getMilliseconds()/1000.0
    def setTtl(self, duration) :
        self._setTtl(Duration(int(1000*duration)))
    __swig_getmethods__["ttl"] = getTtl
    __swig_setmethods__["ttl"] = setTtl
    if _newclass: ttl = property(getTtl, setTtl)

    __swig_getmethods__["user_id"] = getUserId
    __swig_setmethods__["user_id"] = setUserId
    if _newclass: user_id = property(getUserId, setUserId)

    __swig_getmethods__["correlation_id"] = getCorrelationId
    __swig_setmethods__["correlation_id"] = setCorrelationId
    if _newclass: correlation_id = property(getCorrelationId, setCorrelationId)

    __swig_getmethods__["redelivered"] = getRedelivered
    __swig_setmethods__["redelivered"] = setRedelivered
    if _newclass: redelivered = property(getRedelivered, setRedelivered)

    __swig_getmethods__["durable"] = getDurable
    __swig_setmethods__["durable"] = setDurable
    if _newclass: durable = property(getDurable, setDurable)

    __swig_getmethods__["properties"] = getProperties
    if _newclass: properties = property(getProperties)

    def getReplyTo(self) :
        return self._getReplyTo().str()
    def setReplyTo(self, address_str) :
        self._setReplyTo(Address(address_str))
    __swig_getmethods__["reply_to"] = getReplyTo
    __swig_setmethods__["reply_to"] = setReplyTo
    if _newclass: reply_to = property(getReplyTo, setReplyTo)

    def __repr__(self):
        args = []
        for name in ["id", "subject", "user_id", "reply_to",
                     "correlation_id", "priority", "ttl",
                     "durable", "redelivered", "properties",
                     "content_type"] :
            value = getattr(self, name)
            if value : args.append("%s=%r" % (name, value))
        if self.content is not None:
            if args:
                args.append("content=%r" % self.content)
            else:
                args.append(repr(self.content))
        return "Message(%s)" % ", ".join(args)

Message_swigregister = _cqpid.Message_swigregister
Message_swigregister(Message)

def __mul__(*args):
  return _cqpid.__mul__(*args)
__mul__ = _cqpid.__mul__

class EncodingException(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, EncodingException, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, EncodingException, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_EncodingException(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_EncodingException
    __del__ = lambda self : None;
EncodingException_swigregister = _cqpid.EncodingException_swigregister
EncodingException_swigregister(EncodingException)

class Receiver(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Receiver, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Receiver, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_Receiver(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_Receiver
    __del__ = lambda self : None;
    def get(self, *args): return _cqpid.Receiver_get(self, *args)
    def _fetch(self, *args): return _cqpid.Receiver__fetch(self, *args)
    def setCapacity(self, *args): return _cqpid.Receiver_setCapacity(self, *args)
    def getCapacity(self): return _cqpid.Receiver_getCapacity(self)
    def available(self): return _cqpid.Receiver_available(self)
    def unsettled(self): return _cqpid.Receiver_unsettled(self)
    def close(self): return _cqpid.Receiver_close(self)
    def isClosed(self): return _cqpid.Receiver_isClosed(self)
    def getName(self): return _cqpid.Receiver_getName(self)
    def getSession(self): return _cqpid.Receiver_getSession(self)
    __swig_getmethods__["capacity"] = getCapacity
    __swig_setmethods__["capacity"] = setCapacity
    if _newclass: capacity = property(getCapacity, setCapacity)

    __swig_getmethods__["session"] = getSession
    if _newclass: session = property(getSession)

    def fetch(self, timeout=None) :
        if timeout is None :
            return self._fetch()
        else :
            # Python API uses timeouts in seconds,
            # but C++ API uses milliseconds
            return self._fetch(Duration(int(1000*timeout)))

Receiver_swigregister = _cqpid.Receiver_swigregister
Receiver_swigregister(Receiver)

def decode(*args):
  return _cqpid.decode(*args)
decode = _cqpid.decode

def encode(*args):
  return _cqpid.encode(*args)
encode = _cqpid.encode

class Sender(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Sender, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Sender, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_Sender(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_Sender
    __del__ = lambda self : None;
    def _send(self, *args): return _cqpid.Sender__send(self, *args)
    def close(self): return _cqpid.Sender_close(self)
    def setCapacity(self, *args): return _cqpid.Sender_setCapacity(self, *args)
    def getCapacity(self): return _cqpid.Sender_getCapacity(self)
    def unsettled(self): return _cqpid.Sender_unsettled(self)
    def available(self): return _cqpid.Sender_available(self)
    def getName(self): return _cqpid.Sender_getName(self)
    def getSession(self): return _cqpid.Sender_getSession(self)
    def send(self, object, sync=True) :
        if isinstance(object, Message):
            message = object
        else:
            message = Message(object)
        return self._send(message, sync)

    __swig_getmethods__["capacity"] = getCapacity
    __swig_setmethods__["capacity"] = setCapacity
    if _newclass: capacity = property(getCapacity, setCapacity)

    __swig_getmethods__["session"] = getSession
    if _newclass: session = property(getSession)

Sender_swigregister = _cqpid.Sender_swigregister
Sender_swigregister(Sender)

class Session(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Session, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Session, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_Session(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_Session
    __del__ = lambda self : None;
    def close(self): return _cqpid.Session_close(self)
    def commit(self): return _cqpid.Session_commit(self)
    def rollback(self): return _cqpid.Session_rollback(self)
    def _acknowledge_all(self, sync = False): return _cqpid.Session__acknowledge_all(self, sync)
    def _acknowledge_msg(self, *args): return _cqpid.Session__acknowledge_msg(self, *args)
    def acknowledge(self, *args): return _cqpid.Session_acknowledge(self, *args)
    def acknowledgeUpTo(self, *args): return _cqpid.Session_acknowledgeUpTo(self, *args)
    def reject(self, *args): return _cqpid.Session_reject(self, *args)
    def release(self, *args): return _cqpid.Session_release(self, *args)
    def sync(self, block = True): return _cqpid.Session_sync(self, block)
    def getReceivable(self): return _cqpid.Session_getReceivable(self)
    def getUnsettledAcks(self): return _cqpid.Session_getUnsettledAcks(self)
    def nextReceiver(self, *args): return _cqpid.Session_nextReceiver(self, *args)
    def sender(self, *args): return _cqpid.Session_sender(self, *args)
    def receiver(self, *args): return _cqpid.Session_receiver(self, *args)
    def getSender(self, *args): return _cqpid.Session_getSender(self, *args)
    def getReceiver(self, *args): return _cqpid.Session_getReceiver(self, *args)
    def getConnection(self): return _cqpid.Session_getConnection(self)
    def hasError(self): return _cqpid.Session_hasError(self)
    def checkError(self): return _cqpid.Session_checkError(self)
    def acknowledge(self, message=None, disposition=None, sync=True) :
        if disposition :
            raise Exception("SWIG does not support dispositions yet. Use "
                            "Session.reject and Session.release instead")
        if message :
            self._acknowledge_msg(message, sync)
        else :
            self._acknowledge_all(sync)

    __swig_getmethods__["connection"] = getConnection
    if _newclass: connection = property(getConnection)

Session_swigregister = _cqpid.Session_swigregister
Session_swigregister(Session)

class Connection(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Connection, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Connection, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_Connection(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_Connection
    __del__ = lambda self : None;
    def setOption(self, *args): return _cqpid.Connection_setOption(self, *args)
    def open(self): return _cqpid.Connection_open(self)
    def opened(self): return _cqpid.Connection_opened(self)
    def isOpen(self): return _cqpid.Connection_isOpen(self)
    def close(self): return _cqpid.Connection_close(self)
    def createTransactionalSession(self, *args): return _cqpid.Connection_createTransactionalSession(self, *args)
    def createSession(self, *args): return _cqpid.Connection_createSession(self, *args)
    def getSession(self, *args): return _cqpid.Connection_getSession(self, *args)
    def getAuthenticatedUsername(self): return _cqpid.Connection_getAuthenticatedUsername(self)
    # Handle the different options by converting underscores to hyphens.
    # Also, the sasl_mechanisms option in Python has no direct
    # equivalent in C++, so we will translate them to sasl_mechanism
    # when possible.
    def __init__(self, url=None, **options):
        if url:
            args = [url]
        else:
            args = []
        if options :
            if "sasl_mechanisms" in options :
                if ' ' in options.get("sasl_mechanisms",'') :
                    raise Exception(
                        "C++ Connection objects are unable to handle "
                        "multiple sasl-mechanisms")
                options["sasl_mechanism"] = options.pop("sasl_mechanisms")
            args.append(options)
        this = _cqpid.new_Connection(*args)
        try: self.this.append(this)
        except: self.this = this

    def _session(self, *args): return _cqpid.Connection__session(self, *args)
    def session(self, name=None, transactional=False) :
        if name is None :
            name = ''
        return self._session(name, transactional)

    @staticmethod
    def establish(url=None, **options) :
        conn = Connection(url, **options)
        conn.open()
        return conn

Connection_swigregister = _cqpid.Connection_swigregister
Connection_swigregister(Connection)

class FailoverUpdates(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, FailoverUpdates, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, FailoverUpdates, name)
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cqpid.new_FailoverUpdates(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cqpid.delete_FailoverUpdates
    __del__ = lambda self : None;
FailoverUpdates_swigregister = _cqpid.FailoverUpdates_swigregister
FailoverUpdates_swigregister(FailoverUpdates)


def decodeMap(*args):
  return _cqpid.decodeMap(*args)
decodeMap = _cqpid.decodeMap

def decodeList(*args):
  return _cqpid.decodeList(*args)
decodeList = _cqpid.decodeList
# Bring into module scope
UNSPECIFIED = Message.UNSPECIFIED

# This file is compatible with both classic and new-style classes.


