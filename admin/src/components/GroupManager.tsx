import { useState, useEffect } from 'react';
import { Table, Button, Title, Card, Group, TextInput, Stack, ActionIcon, Modal, Select, Text, Box } from '@mantine/core';
import { IconTrash, IconPlus, IconUsersGroup } from '@tabler/icons-react';
import api from '../api';
import type { Group as GroupType, User } from '../api';
import { notifications } from '@mantine/notifications';

export const GroupManager = () => {
  const [groups, setGroups] = useState<GroupType[]>([]);
  const [users, setUsers] = useState<User[]>([]);
  const [newGroupName, setNewGroupName] = useState('');
  const [newGroupDesc, setNewGroupDesc] = useState('');
  const [selectedGroupId, setSelectedGroupId] = useState<number | null>(null);
  const [isMemberModalOpen, setIsMemberModalOpen] = useState(false);
  const [userToAdd, setUserToAdd] = useState<string | null>(null);

  const fetchData = async () => {
    try {
      const [groupsRes, usersRes] = await Promise.all([
        api.get('/groups'),
        api.get('/users')
      ]);
      setGroups(groupsRes.data);
      setUsers(usersRes.data);
    } catch (e) {
      notifications.show({ title: 'Error', message: 'Failed to fetch data', color: 'red' });
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  const handleAddGroup = async () => {
    if (!newGroupName) return;
    try {
      await api.post('/groups', { name: newGroupName, description: newGroupDesc });
      setNewGroupName('');
      setNewGroupDesc('');
      fetchData();
      notifications.show({ title: 'Success', message: 'Group created', color: 'green' });
    } catch (e) {
      notifications.show({ title: 'Error', message: 'Failed to create group', color: 'red' });
    }
  };

  const handleDeleteGroup = async (id: number) => {
    if (!window.confirm('Delete this group?')) return;
    try {
      await api.delete(`/groups/${id}`);
      fetchData();
      notifications.show({ title: 'Success', message: 'Group deleted', color: 'green' });
    } catch (e) {
      notifications.show({ title: 'Error', message: 'Failed to delete group', color: 'red' });
    }
  };

  const handleAddMember = async () => {
    if (!selectedGroupId || !userToAdd) return;
    try {
      await api.post(`/groups/${selectedGroupId}/members/${userToAdd}`);
      setUserToAdd(null);
      fetchData();
      notifications.show({ title: 'Success', message: 'User added to group', color: 'green' });
    } catch (e) {
      notifications.show({ title: 'Error', message: 'Failed to add user', color: 'red' });
    }
  };

  const handleRemoveMember = async (userId: number) => {
    if (!selectedGroupId) return;
    try {
      await api.delete(`/groups/${selectedGroupId}/members/${userId}`);
      fetchData();
      notifications.show({ title: 'Success', message: 'User removed from group', color: 'green' });
    } catch (e) {
      notifications.show({ title: 'Error', message: 'Failed to remove user', color: 'red' });
    }
  };

  const selectedGroup = groups.find(g => g.id === selectedGroupId);

  return (
    <Stack>
      <Card withBorder padding="lg" radius="md">
        <Title order={3} mb="md">Add New Group</Title>
        <Group align="flex-end">
          <TextInput
            label="Group Name"
            placeholder="e.g. VIPs"
            value={newGroupName}
            onChange={(e) => setNewGroupName(e.currentTarget.value)}
            style={{ flex: 1 }}
          />
          <TextInput
            label="Description"
            placeholder="Optional description"
            value={newGroupDesc}
            onChange={(e) => setNewGroupDesc(e.currentTarget.value)}
            style={{ flex: 2 }}
          />
          <Button leftSection={<IconPlus size={16} />} onClick={handleAddGroup}>
            Add Group
          </Button>
        </Group>
      </Card>

      <Card withBorder padding="lg" radius="md">
        <Title order={3} mb="md">Existing Groups</Title>
        <Table verticalSpacing="sm">
          <Table.Thead>
            <Table.Tr>
              <Table.Th>Name</Table.Th>
              <Table.Th>Description</Table.Th>
              <Table.Th align="right">Actions</Table.Th>
            </Table.Tr>
          </Table.Thead>
          <Table.Tbody>
            {groups.map((group) => (
              <Table.Tr key={group.id}>
                <Table.Td fw={500}>{group.name}</Table.Td>
                <Table.Td>{group.description}</Table.Td>
                <Table.Td>
                  <Group justify="flex-end">
                    <Button 
                      variant="light" 
                      size="xs" 
                      leftSection={<IconUsersGroup size={14} />}
                      onClick={() => {
                        setSelectedGroupId(group.id);
                        setIsMemberModalOpen(true);
                      }}
                    >
                      Manage Members
                    </Button>
                    <ActionIcon color="red" variant="subtle" onClick={() => handleDeleteGroup(group.id)}>
                      <IconTrash size={16} />
                    </ActionIcon>
                  </Group>
                </Table.Td>
              </Table.Tr>
            ))}
          </Table.Tbody>
        </Table>
      </Card>

      <Modal 
        opened={isMemberModalOpen} 
        onClose={() => setIsMemberModalOpen(false)} 
        title={`Manage members of ${selectedGroup?.name || 'group'}`}
        size="md"
      >
        <Stack gap="md">
          <Box>
            <Text size="sm" fw={500} mb="xs">Current members:</Text>
            {selectedGroup?.members && selectedGroup.members.length > 0 ? (
              <Table variant="vertical" withTableBorder>
                <Table.Tbody>
                  {selectedGroup.members.map(member => (
                    <Table.Tr key={member.id}>
                      <Table.Td>
                        <Text size="sm">{member.name} {member.surname} ({member.nickname})</Text>
                      </Table.Td>
                      <Table.Td align="right">
                        <ActionIcon color="red" variant="subtle" size="sm" onClick={() => handleRemoveMember(member.id)}>
                          <IconTrash size={14} />
                        </ActionIcon>
                      </Table.Td>
                    </Table.Tr>
                  ))}
                </Table.Tbody>
              </Table>
            ) : (
              <Text size="xs" c="dimmed" fs="italic">No members in this group yet.</Text>
            )}
          </Box>

          <Box>
            <Text size="sm" fw={500} mb="xs">Add a user to this group:</Text>
            <Group align="flex-end">
              <Select
                placeholder="Select user"
                data={users
                  .filter(u => !selectedGroup?.members?.some(m => m.id === u.id))
                  .map(u => ({ value: u.id.toString(), label: `${u.name} (${u.nickname})` }))
                }
                value={userToAdd}
                onChange={setUserToAdd}
                searchable
                style={{ flex: 1 }}
              />
              <Button onClick={handleAddMember}>Add</Button>
            </Group>
          </Box>
        </Stack>
      </Modal>
    </Stack>
  );
};
