/***
  This file is part of PulseAudio.

  Copyright 2009 Lennart Poettering

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <sys/mman.h>

#include <pulsecore/memtrap.h>
#include <pulsecore/core-util.h>

int main(int argc, char *argv[]) {
    void *p;
    int fd;
    pa_memtrap *m;

    pa_log_set_level(PA_LOG_DEBUG);

    /* Create the memory map */
    pa_assert_se((fd = open("sigbus-test-map", O_RDWR|O_TRUNC|O_CREAT, 0660)) >= 0);
    pa_assert_se(unlink("sigbus-test-map") == 0);
    pa_assert_se(ftruncate(fd, PA_PAGE_SIZE) >= 0);
    pa_assert_se((p = mmap(NULL, PA_PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) != MAP_FAILED);

    /* Register memory map */
    m = pa_memtrap_add(p, PA_PAGE_SIZE);

    /* Use memory map */
    pa_snprintf(p, PA_PAGE_SIZE, "This is a test that should work fine.");

    /* Verify memory map */
    pa_log("Let's see if this worked: %s", (char*) p);
    pa_log("And memtrap says it is good: %s", pa_yes_no(pa_memtrap_is_good(m)));

    /* Invalidate mapping */
    pa_assert_se(ftruncate(fd, 0) >= 0);

    /* Use memory map */
    pa_snprintf(p, PA_PAGE_SIZE, "This is a test that should fail but get caught.");

    /* Verify memory map */
    pa_log("Let's see if this worked: %s", (char*) p);
    pa_log("And memtrap says it is good: %s", pa_yes_no(pa_memtrap_is_good(m)));

    pa_memtrap_remove(m);
    munmap(p, PA_PAGE_SIZE);

    return 0;
}
