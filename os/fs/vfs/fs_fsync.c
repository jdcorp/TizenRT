/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * fs/vfs/fs_fsync.c
 *
 *   Copyright (C) 2007-2009, 2013-2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include <tinyara/sched.h>
#include <tinyara/cancelpt.h>
#include <tinyara/fs/fs.h>

#include "inode/inode.h"

#ifndef CONFIG_DISABLE_MOUNTPOINT

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Variables
 ****************************************************************************/

/****************************************************************************
 * Public Variables
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_fsync
 *
 * Description:
 *   Equivalent to the standard fsync() function except that is accepts a
 *   struct file instance instead of a file descriptor.  Currently used
 *   only by aio_fsync();
 *
 ****************************************************************************/

int file_fsync(FAR struct file *filep)
{
	struct inode *inode;
	int ret;

	/* Was this file opened for write access? */

	if ((filep->f_oflags & O_WROK) == 0) {
		ret = EBADF;
		goto errout;
	}

	/* Is this inode a registered mountpoint? Does it support the
	 * sync operations may be relevant to device drivers but only
	 * the mountpoint operations vtable contains a sync method.
	 */

	inode = filep->f_inode;
	if (!inode || !INODE_IS_MOUNTPT(inode) || !inode->u.i_mops || !inode->u.i_mops->sync) {
		ret = EINVAL;
		goto errout;
	}

	/* Yes, then tell the mountpoint to sync this file */

	ret = inode->u.i_mops->sync(filep);
	if (ret >= 0) {
		return OK;
	}

	ret = -ret;

errout:
	set_errno(ret);
	return ERROR;
}

/****************************************************************************
 * Name: fsync
 *
 * Description:
 *   This func simply binds inode sync methods to the sync system call.
 *
 ****************************************************************************/

int fsync(int fd)
{
	FAR struct file *filep;
	int ret;

	/* fsync() is a cancellation point */
	(void)enter_cancellation_point();

	/* Get the file structure corresponding to the file descriptor. */

	ret = fs_getfilep(fd, &filep);
	if (ret < 0) {
		goto errout;
	}

	DEBUGASSERT(filep != NULL);

	/* Perform the fsync operation */

	ret = file_fsync(filep);
	leave_cancellation_point();
	return ret;
errout:
	leave_cancellation_point();
	set_errno(-ret);
	return ERROR;
}

#endif							/* !CONFIG_DISABLE_MOUNTPOINT */
