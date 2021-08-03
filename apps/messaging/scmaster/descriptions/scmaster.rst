scmaster is the implementation of the :ref:`messaging <concepts_messaging>`
mediator.


Message Groups
==============

scmaster provides the :ref:`message groups <messaging-groups>`. Configure

* :confval:`defaultGroups`: Add the groups which can be used by all queues.
* :confval:`queues.$name.groups`: Add all groups which are used by the given queue.
  Adding any values ignores all values of :confval:`defaultGroups`.


Queues
======

scmaster provides :ref:`queues <messaging-queues>` for separating the processing.
Typically, the default queue *production* is used. To add new queues

#. Define a new queue by adding a new profile with some name,
#. Configure the profile parameters :confval:`queues.$name.*,
#. Register the queue in :confval:`queues`.


Scheme
======

scmaster provides unsecured and secured connection which is addressed by the
scheme values *scmp* and *scmps*, respectively, in :confval:`connection.server`
when connecting to the messaging.
Read the :ref:`concepts section <messaging-scheme>` for more details. *scmps*
is in use when configuring :confval:`interface.ssl.bind`.


Database Access
===============

scmaster reads from and writes to the database and reports the database connection
to the clients of the messaging system (compare with the :ref:`concepts section <messaging-db>`).

The database is configured per queue.
When running all |scname| modules on a single machine, the read and write
parameters are typically configured with *localhost* as a *host name*.

Example: ::

   queues.production.processors.messages.dbstore.read = sysop:sysop@localhost/seiscomp
   queues.production.processors.messages.dbstore.write = sysop:sysop@localhost/seiscomp

However, if the clients are
located on machines different from the messaging, the *host name* of the read parameter
must be available on the client machine and the client machine must be able to
connect to the host with its name. If the database is on the same machine as the
messaging, the *host name* of the write connection typically remains *localhost*.

Example for connecting clients on computerB to the messaging on computerA (compare
with the :ref:`concepts section <messaging-distribution>`).

* Configuration of scmaster on computerA: ::

     queues.production.processors.messages.dbstore.read = sysop:sysop@computerA/seiscomp
     queues.production.processors.messages.dbstore.write = sysop:sysop@localhost/seiscomp

* Global configuration of client on computerB: ::

     connection.server = computerA/production
