#ifndef __JPOD_CONTAINER_PARAMETERS__
#define __JPOD_CONTAINER_PARAMETERS__

#include <string>
#include <vector>
namespace containers {

  struct MountPoint {
    std::string from;
    std::string to;
  };

  struct Composition {
    std::string snapshot_path;
    std::string hostname;
    std::string ip_v4_address;
    std::string ip_v6_address;
    std::vector<MountPoint> mountPoints;
  };

  enum class BaseOS {
    LINUX,
    FREE_BSD
  };

  enum class CommandType {
    FROM,
    COPY,
    RUN,
    ENV,
    CMD
  };

  enum class UpdateType {
    PRE,
    POST
  };

  enum class ContainerState {
    CREATED,
    UPDATED,
    STARTED,
    STOPPED
  };

  struct ContainerInformation {
    std::string identifier;
    std::string ip_v4;
    std::string ip_v6;
    std::string hostname;
  };

  struct ContainerReport {

  public:
    ContainerReport(const ContainerState state, const std::string &identifier, const std::string &log) : state(state),
                                                                                                         identifier(identifier),
                                                                                                         log(log) {}
    bool isCreated() {
      return state == ContainerState::CREATED;
    }

    bool isUpdated() {
      return state == ContainerState::UPDATED;
    }

    bool isStarted() {
      return state == ContainerState::STARTED;
    }

    bool isStopped() {
      return state == ContainerState::STOPPED;
    }

    std::string getLog() {
      return log;
    }

    std::string getIdentifier() {
      return identifier;
    }

  private:
    ContainerState state;
    std::string identifier;
    std::string log;
  };
}; // namespace containers

#endif // __JPOD_CONTAINER_PARAMETERS__