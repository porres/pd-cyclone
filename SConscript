import glob
import os
import re
Import('env prefix')

env.Append(CPPPATH = 'shared')

miXed_shared = []
miXed_shared.extend(glob.glob('shared/common/*.c'))
miXed_shared.extend(Split('shared/unstable/fringe.c shared/unstable/forky.c shared/unstable/fragile.c shared/unstable/loader.c'))

hammer_src = []
hammer_src.extend(miXed_shared)
hammer_src.extend(glob.glob('shared/hammer/*.c'))
hammer_src.extend(glob.glob('cyclone/hammer/*.c'))
hammer = env.SharedLibrary(target = 'hammer', source = hammer_src)
env.Alias('install', env.Install(os.path.join(prefix, 'extra'), hammer))

sickle_src = []
sickle_src.extend(miXed_shared)
sickle_src.extend(glob.glob('shared/sickle/*.c'))
sickle_src.extend(glob.glob('cyclone/sickle/*.c'))
sickle = env.SharedLibrary(target = 'sickle', source = sickle_src)
env.Alias('install', env.Install(os.path.join(prefix, 'extra'), sickle))


env.Alias('install', env.Install(os.path.join(prefix, 'doc/5.reference/'), glob.glob('doc/help/*/*.pd')))

Default(miXed_shared)
Default(hammer)
Default(sickle)
