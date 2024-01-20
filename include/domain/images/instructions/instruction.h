#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_INSTRUCTION__

#include <string>
namespace domain::images::instructions
{
   class instruction_listener;
   class instruction
   {
   public:
      virtual ~instruction() = default;
      virtual void execute() = 0;

   protected:
      instruction(std::string name, instruction_listener &listener) : name(name), listener(listener) {}

   protected:
      std::string name;
      instruction_listener &listener;
   };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_INSTRUCTION__