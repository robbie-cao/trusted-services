Contributing
============

Reporting Security Issues
-------------------------

Please follow the directions of the `Trusted Firmware Security Center`_

Getting Started
---------------

- Make sure you have a GitHub account and you are logged on `developer.trustedfirmware.org`_.
- Send an email to the |TS_MAIL_LIST| about your work. This gives everyone
  visibility of whether others are working on something similar.
- Clone the |TS_REPO| on your own machine.
- Create a local topic branch based on ``main`` branch of the |TS_REPO|.

Making Changes
--------------

- Make commits of logical units. See these general `Git guidelines`_ for contributing to a project.
- Follow the :ref:`Coding Style & Guidelines`.
- Keep the commits on topic. If you need to fix another bug or make another enhancement, please create a separate
  change.
- Avoid long commit series. If you do have a long series, consider whether some
  commits should be squashed together or addressed in a separate topic.
- Make sure your commit messages are in the proper format. Please keel the 50/72 rule (for details see `Tim Popes blog entry`_.)
- Where appropriate, please update the documentation.

   - Consider which documents or other in-source documentation needs updating.
   - Ensure that each changed file has the correct copyright and license information. Files that entirely consist of
     contributions to this project should have a copyright notice and BSD-3-Clause SPDX license identifier of the form
     as shown in :ref:`license`. Example copyright and license comment blocks are shown in :ref:`Coding Style & Guidelines`.
     Files that contain changes to imported Third Party IP files should retain their original copyright and license
     notices. For significant contributions you may add your own copyright notice in following format::

        Portions copyright (c) [XXXX-]YYYY, <OWNER>. All rights reserved.

     where XXXX is the year of first contribution (if different to YYYY) and YYYY is the year of most recent
     contribution. *<OWNER>* is your name or your company name.
   - For any change, ensure that YYYY is updated if a contribution is made in a year more recent than the previous YYYY.
   - If you are submitting new files that you intend to be the technical sub-maintainer for (for example, a new platform
     port), then also update the :ref:`maintainers` file.
   - For topics with multiple commits, you should make all documentation changes (and nothing else) in the last commit
     of the series. Otherwise, include the documentation changes within the single commit.

- Please test your changes.

Submitting Changes
------------------

- Ensure that each commit in the series has at least one ``Signed-off-by:`` line, using your real name and email
  address. The names in the ``Signed-off-by:`` and ``Author:`` lines must match. If anyone else contributes to the
  commit, they must also add their own ``Signed-off-by:`` line. By adding this line the contributor certifies the
  contribution is made under the terms of the :download:`Developer Certificate of Origin <../../dco.txt>`.

  More details may be found in the `Gerrit Signed-off-by Lines guidelines`_.

- Ensure that each commit also has a unique ``Change-Id:`` line. If you have cloned the repository with the "`Clone with
  commit-msg hook`" clone method, this should already be the case.

  More details may be found in the `Gerrit Change-Ids documentation`_.

- Submit your changes for review at https://review.trustedfirmware.org targeting the ``integration`` branch.

  - The changes will then undergo further review and testing by the :ref:`maintainers`. Any review comments will be made
    directly on your patch. This may require you to do some rework.

  Refer to the `Gerrit Uploading Changes documentation`_ for more details.

- When the changes are accepted, the :ref:`maintainers` will integrate them.

  - Typically, the :ref:`maintainers` will merge the changes into the ``integration`` branch.
  - If the changes are not based on a sufficiently-recent commit, or if they cannot be automatically rebased, then the
    :ref:`maintainers` may rebase it on the ``main`` branch or ask you to do so.
  - After final integration testing, the changes will make their way into the ``main`` branch. If a problem is found
    during integration, the merge commit will be removed from the ``integration`` branch and the :ref:`maintainers` will
    ask you to create a new patch set to resolve the problem.

--------------

.. _developer.trustedfirmware.org: https://developer.trustedfirmware.org
.. _Git guidelines: http://git-scm.com/book/ch5-2.html
.. _Gerrit Uploading Changes documentation: https://review.trustedfirmware.org/Documentation/user-upload.html
.. _Gerrit Signed-off-by Lines guidelines: https://review.trustedfirmware.org/Documentation/user-signedoffby.html
.. _Gerrit Change-Ids documentation: https://review.trustedfirmware.org/Documentation/user-changeid.html
.. _`Tim Popes blog entry`: https://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
.. _`Trusted Firmware Security Center`: https://developer.trustedfirmware.org/w/collaboration/security_center/

*Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.*

SPDX-License-Identifier: BSD-3-Clause
