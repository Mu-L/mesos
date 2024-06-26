// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __PERF_EVENT_HPP__
#define __PERF_EVENT_HPP__

#include <set>
#include <string>

#include <mesos/resources.hpp>
#include <process/clock.hpp>
#include <process/future.hpp>
#include <process/owned.hpp>
#include <process/time.hpp>
#include <stout/hashmap.hpp>

#include "slave/containerizer/mesos/isolators/cgroups2/constants.hpp"
#include "slave/containerizer/mesos/isolators/cgroups2/controller.hpp"
#include "slave/flags.hpp"

namespace mesos {
namespace internal {
namespace slave {

class PerfEventControllerProcess : public ControllerProcess
{
public:
  static Try<process::Owned<ControllerProcess>> create(const Flags& flags);

  ~PerfEventControllerProcess() override = default;

  std::string name() const override;

  process::Future<Nothing> prepare(
    const ContainerID& containerId,
    const std::string& cgroup,
    const mesos::slave::ContainerConfig& containerConfig) override;

  process::Future<Nothing> recover(
    const ContainerID& containerId, const std::string& cgroup) override;

  process::Future<ResourceStatistics> usage(
    const ContainerID& containerId, const std::string& cgroup) override;

  process::Future<Nothing> cleanup(
    const ContainerID& containerId, const std::string& cgroup) override;

protected:
  void initialize() override;

private:
  PerfEventControllerProcess(
    const Flags& flags, const std::set<std::string>& _events);

  struct Info
  {
    Info(const std::string& _cgroup) : cgroup(_cgroup)
    {
      // Ensure the initial statistics include the required fields.
      // Note the duration is set to zero to indicate no sampling has
      // taken place. This empty sample will be returned from usage()
      // until the first true sample is obtained.
      statistics.set_timestamp(process::Clock::now().secs());
      statistics.set_duration(Seconds(0).secs());
    }

    const std::string cgroup;
    PerfStatistics statistics;
  };

  void sample();

  void _sample(
    const process::Time& next,
    const process::Future<hashmap<std::string, PerfStatistics>>& statistics);

  // Set of events to sample.
  std::set<std::string> events;

  // Stores cgroups associated information for container.
  hashmap<ContainerID, process::Owned<Info>> infos;
};


} // namespace slave {
} // namespace internal {
} // namespace mesos {


#endif // __PERF_EVENT_HPP__