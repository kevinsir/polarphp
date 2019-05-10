// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2019 polarphp software foundation
// Copyright (c) 2017 - 2019 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://polarphp.org/LICENSE.txt for license information
// See https://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by polarboy on 2019/02/07.

#include "ModuleCycleHooks.h"
#include "polarphp/vm/lang/Module.h"
#include <iostream>

namespace php {

void module_startup_hook()
{
   std::cout << "stdlib module startup ... " << std::endl;
}

void module_process_startup_hook()
{
   std::cout << "stdlib module process startup ... " << std::endl;
}

void module_process_shutdown_hook()
{
   std::cout << "stdlib module process shutdown ... " << std::endl;
}

void module_shutdown_hook()
{
   std::cout << "stdlib module shutdown ... " << std::endl;
}

void register_module_cycle_hooks(Module &module)
{
//   module.setStartupHandler(module_startup_hook);
//   module.setRequestStartupHandler(module_process_startup_hook);
//   module.setRequestShutdownHandler(module_process_shutdown_hook);
//   module.setShutdownHandler(module_shutdown_hook);
}

} // php
