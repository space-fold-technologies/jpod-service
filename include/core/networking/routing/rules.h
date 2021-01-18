#ifndef __JPOD__NETWORKING_ROUTING__
#define __JPOD__NETWORKING_ROUTING__
#include <array>
#include <net/if.h>
#include <net/pfvar.h>
#include <string>
#include <vector>

namespace networking::routing {

  enum class AddressFamily {
    ANY = 0,
    IP_V4 = AF_INET,
    IP_V6 = AF_INET6
  };

  enum class Protocol {
    TCP = IPPROTO_TCP,
    UDP = IPPROTO_UDP,
    ICMP = IPPROTO_ICMP,
    ANY = 0
  };

  enum class Action {
    PASS = PF_PASS,
    DROP = PF_DROP,
    SCRUB = PF_SCRUB,
    NO_SCRUB = PF_NOSCRUB,
    RE_DIRECT = PF_RDR,
    NAT = PF_NAT,
    NO_NAT = PF_NONAT,
    BI_NAT = PF_BINAT,
    NO_BI_NAT = PF_NOBINAT,
  };

  enum class Direction {
    IN = PF_IN,
    OUT = PF_OUT,
    IN_OUT = PF_INOUT
  };

  enum class State {
    NONE = 0,
    NORMAL = PF_STATE_NORMAL,
    MODULATE = PF_STATE_MODULATE,
    SYN_PROXY = PF_STATE_SYNPROXY
  };

  enum class PortOperation {
    RANGE = PF_OP_RRG,
    NEGLECT = PF_OP_NE,
    OUT_OFF = PF_OP_XRG,
    LESS_THAN_OR_EQUAL = PF_OP_LE,
    LESS_THAN = PF_OP_LT,
    BETWEEN = PF_OP_IRG,
    GREATER_THAN_OR_EQUAL = PF_OP_GE,
    GREATER_THAN = PF_OP_GT,
    NONE = PF_OP_NONE
  };

  class Rule {
    friend class RuleBuilder;

  public:
    const pfioc_rule &content() const { return this->_rule; };
    Action action() const;
    Protocol protocol() const { return static_cast<Protocol>(_rule.rule.proto); };
    Direction direction() const { return static_cast<Direction>(_rule.rule.direction); };
    AddressFamily family() const { return static_cast<AddressFamily>(_rule.rule.af); };
    State state() const { return static_cast<State>(_rule.rule.keep_state); };
    bool log() const { return _rule.rule.log == 1 ? true : false; };
    bool quick() const { return _rule.rule.quick == 1 ? true : false; };

  private:
    Rule(pfioc_rule _rule) : _rule(_rule) {}
    pfioc_rule _rule;
  };

  class RuleBuilder final {
    friend class Rule;

  public:
    RuleBuilder() {}
    ~RuleBuilder() {}
    auto static builder() -> RuleBuilder {
      return RuleBuilder();
    }
    auto action(Action action) -> RuleBuilder & {
      this->_rule.rule.action = static_cast<u_int8_t>(action);
      return *this;
    };
    auto protocol(Protocol protocol) -> RuleBuilder & {
      this->_rule.rule.proto = static_cast<u_int8_t>(protocol);
      return *this;
    };
    auto direction(Direction direction) -> RuleBuilder & {
      _rule.rule.direction = static_cast<u_int8_t>(direction);
      return *this;
    };

    auto log() -> RuleBuilder & {
      _log = true;
      return *this;
    };
    auto state(State state) -> RuleBuilder & {
      this->_rule.rule.keep_state = static_cast<u_int8_t>(state);
      return *this;
    };
    auto quick() -> RuleBuilder & {
      _quick = true;
      return *this;
    };

    auto source(const std::string &address, int port) -> RuleBuilder & {
      this->_source_address = address;
      this->_source_ports[0] = port;
      this->_source_port_operation = PortOperation::NONE;
      return *this;
    }

    auto destination(const std::string &address, int port) -> RuleBuilder & {
      this->_destination_address = address;
      this->_destination_ports[0] = port;
      this->_destination_port_operation = PortOperation::NONE;
      return *this;
    }

    auto source(const std::string &address, int port_from, int port_to, PortOperation port_operation) -> RuleBuilder & {
      this->_source_address = address;
      this->_source_ports[0] = port_from;
      this->_source_ports[1] = port_to;
      this->_source_port_operation = port_operation;
      return *this;
    }

    auto destination(const std::string &address, int port_from, int port_to, PortOperation port_operation) -> RuleBuilder & {
      this->_destination_address = address;
      this->_destination_ports[0] = port_from;
      this->_destination_ports[1] = port_to;
      this->_destination_port_operation = port_operation;
      return *this;
    }

    auto source_interface(const std::string &name) -> RuleBuilder & {
      this->_source_interface = name;
      return *this;
    }

    auto destination_interface(const std::string &name) -> RuleBuilder & {
      this->_destination_interface = name;
      return *this;
    }

    auto address_family(AddressFamily address_family) -> RuleBuilder & {
      this->_rule.rule.af = static_cast<sa_family_t>(address_family);
      return *this;
    }

    Rule build() {
      if (_source_ports.size() > 0) {
        std::memcpy(_rule.rule.src.port, _source_ports.data(), _source_ports.size());
        _rule.rule.src.port_op = static_cast<u_int8_t>(_source_port_operation);
      }

      if (_destination_ports.size() > 0) {
        std::memcpy(_rule.rule.dst.port, _destination_ports.data(), _destination_ports.size());
        _rule.rule.dst.port_op = static_cast<u_int8_t>(_destination_port_operation);
      }

      if (!_source_interface.empty()) {
        add_interface(_rule.rule.src, _source_interface);
      }

      if (!_destination_interface.empty()) {
        add_interface(_rule.rule.dst, _destination_interface);
      }
      if (!_source_address.empty()) {
        if (_source_address.find(":") > 0) {
          address_family(AddressFamily::IP_V6);
        } else {
          address_family(AddressFamily::IP_V4);
        }
        add_address(_rule.rule.src, _source_address);
      }

      if (!_destination_address.empty()) {
        add_address(_rule.rule.dst, _destination_address);
      }
      _rule.rule.log = _log ? 1 : 0;
      _rule.rule.quick = _quick ? 1 : 0;
      return Rule(std::move(_rule));
    }

  private:
    auto add_interface(struct pf_rule_addr &address_rule, const std::string &name) -> void {
      std::memcpy(address_rule.addr.v.ifname, name.data(), IFNAMSIZ);
    }

    auto convert_address(const std::string &ip_address) -> u_int32_t {
      int a, b, c, d;

      sscanf(ip_address.c_str(), "%i.%i.%i.%i", &a, &b, &c, &d);
      return htonl(a << 24 | b << 16 | c << 8 | d);
    }

    auto add_address(struct pf_rule_addr &address_rule, const std::string &ip_address) -> void {
      add_address(address_rule, ip_address, "255.255.255.0");
    }

    auto add_address(struct pf_rule_addr &address_rule, const std::string &ip_address, const std::string &netmask) -> void {
      if (ip_address.find(":") > 0) {
        // We don't know how to support IPv6
        // address_rule.addr.v.a.addr.v6.in6_ad
      } else {
        address_rule.addr.v.a.addr.v4.s_addr = convert_address(ip_address);
        address_rule.addr.v.a.mask.v4.s_addr = convert_address(netmask);
      }

      address_rule.addr.type = PF_ADDR_ADDRMASK;
    }
    pfioc_rule _rule;
    bool _log;
    bool _quick;
    std::string _source_address;
    std::string _destination_address;
    std::array<int, 2> _source_ports;
    PortOperation _source_port_operation;
    std::array<int, 2> _destination_ports;
    PortOperation _destination_port_operation;
    std::string _source_interface;
    std::string _destination_interface;
  };
  // We will create a rule using the builder pattern

} // namespace networking::routing
#endif // __JPOD__NETWORKING_ROUTING__