#ifndef __JPOD__NETWORKING__VNET__
#define __JPOD__NETWORKING__VNET__
#include <functional>
#include <libifconfig.h>
#include <memory>
namespace networking::vnet {
  template <typename T>
  using configurer_ptr = std::unique_ptr<T, std::function<void(T *)>>;
  class Configurer {
  public:
    Configurer();
    ~Configurer();
    //https://github.com/Savagedlight/libifconfig/blob/master/examples/ifcreate.c
  private:
    ifconfig_handle_t *lifh; //
  };
}; // namespace networking::vnet

#endif // __JPOD__NETWORKING__VNET__