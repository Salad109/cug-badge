import { useEffect, useState } from 'react';
import { Table, Text, ActionIcon, Title, Card, Group, Button } from '@mantine/core';
import { IconTrash, IconPencil, IconPlus } from '@tabler/icons-react';
import api from '../api';
import type { User } from '../api';
import { notifications } from '@mantine/notifications';
import { UserModal } from './UserModal';

export const UserList = () => {
  const [users, setUsers] = useState<User[]>([]);
  const [loading, setLoading] = useState(true);
  const [modalOpened, setModalOpened] = useState(false);
  const [editingUser, setEditingUser] = useState<User | null>(null);

  const fetchUsers = async () => {
    try {
      const response = await api.get('/users');
      setUsers(response.data);
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to fetch users', color: 'red' });
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchUsers();
  }, []);

  const handleUnlink = async (mac: string) => {
    if (!window.confirm(`Are you sure you want to unlink and DELETE user with MAC ${mac}?`)) return;
    try {
      await api.post(`/badges/${mac}/unlink`);
      notifications.show({ title: 'Success', message: 'User unlinked and deleted', color: 'green' });
      fetchUsers();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to unlink user', color: 'red' });
    }
  };

  const handleSaveUser = async (userData: Partial<User>) => {
    try {
      if (editingUser) {
        await api.put(`/users/${editingUser.id}`, userData);
        notifications.show({ title: 'Success', message: 'User updated', color: 'green' });
      } else {
        await api.post('/users', userData);
        notifications.show({ title: 'Success', message: 'User created', color: 'green' });
      }
      setModalOpened(false);
      fetchUsers();
    } catch (error: any) {
      const msg = error.response?.status === 409 ? 'MAC address already in use' : 'Failed to save user';
      notifications.show({ title: 'Error', message: msg, color: 'red' });
    }
  };

  const rows = users.map((user) => (
    <Table.Tr key={user.id}>
      <Table.Td>{user.name} {user.surname}</Table.Td>
      <Table.Td>{user.nickname}</Table.Td>
      <Table.Td>{user.macAddress || <Text c="dimmed" fs="italic" size="sm">Not assigned</Text>}</Table.Td>
      <Table.Td>{user.role}</Table.Td>
      <Table.Td>
        <Group gap="xs">
          <ActionIcon color="blue" variant="subtle" onClick={() => { setEditingUser(user); setModalOpened(true); }}>
            <IconPencil size={16} />
          </ActionIcon>
          <ActionIcon color="red" variant="subtle" onClick={() => handleUnlink(user.macAddress)} title="Unlink & Delete" disabled={!user.macAddress}>
            <IconTrash size={16} />
          </ActionIcon>
        </Group>
      </Table.Td>
    </Table.Tr>
  ));

  return (
    <>
      <Card withBorder padding="lg" radius="md">
        <Group justify="space-between" mb="md">
          <Title order={2}>User Management</Title>
          <Button leftSection={<IconPlus size={16} />} onClick={() => { setEditingUser(null); setModalOpened(true); }}>
            Add User
          </Button>
        </Group>
        <Table highlightOnHover>
          <Table.Thead>
            <Table.Tr>
              <Table.Th>Name</Table.Th>
              <Table.Th>Nickname</Table.Th>
              <Table.Th>MAC Address</Table.Th>
              <Table.Th>Role</Table.Th>
              <Table.Th>Actions</Table.Th>
            </Table.Tr>
          </Table.Thead>
          <Table.Tbody>{rows}</Table.Tbody>
        </Table>
        {users.length === 0 && !loading && <Text ta="center" mt="md" c="dimmed">No users found.</Text>}
      </Card>

      <UserModal 
        opened={modalOpened} 
        onClose={() => setModalOpened(false)} 
        onSubmit={handleSaveUser}
        initialData={editingUser}
      />
    </>
  );
};
