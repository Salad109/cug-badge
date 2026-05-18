import { useEffect, useState } from 'react';
import { Table, Button, TextInput, Group, ActionIcon, Title, Card, Stack } from '@mantine/core';
import { IconTrash, IconPlus } from '@tabler/icons-react';
import api from '../api';
import type { Sector } from '../api';
import { notifications } from '@mantine/notifications';

export const SectorManager = () => {
  const [sectors, setSectors] = useState<Sector[]>([]);
  const [newName, setNewName] = useState('');
  const [newUrl, setNewUrl] = useState('');

  const fetchSectors = async () => {
    try {
      const response = await api.get('/sectors');
      setSectors(response.data);
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to fetch sectors', color: 'red' });
    }
  };

  useEffect(() => {
    fetchSectors();
  }, []);

  const handleAdd = async () => {
    if (!newName || !newUrl) return;
    try {
      await api.post('/sectors', { name: newName, gatewayUrl: newUrl });
      notifications.show({ title: 'Success', message: 'Sector added', color: 'green' });
      setNewName('');
      setNewUrl('');
      fetchSectors();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to add sector', color: 'red' });
    }
  };

  const handleDelete = async (id: number) => {
    try {
      await api.delete(`/sectors/${id}`);
      notifications.show({ title: 'Success', message: 'Sector deleted', color: 'green' });
      fetchSectors();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to delete sector', color: 'red' });
    }
  };

  const rows = sectors.map((sector) => (
    <Table.Tr key={sector.id}>
      <Table.Td>{sector.name}</Table.Td>
      <Table.Td>{sector.gatewayUrl}</Table.Td>
      <Table.Td>
        <ActionIcon color="red" onClick={() => handleDelete(sector.id)}>
          <IconTrash size={16} />
        </ActionIcon>
      </Table.Td>
    </Table.Tr>
  ));

  return (
    <Stack>
      <Card withBorder padding="lg" radius="md">
        <Title order={2} mb="md">Add New Sector (LoRa Gateway)</Title>
        <Group align="flex-end">
          <TextInput
            label="Sector Name"
            placeholder="e.g. Main Hall"
            value={newName}
            onChange={(e) => setNewName(e.currentTarget.value)}
            style={{ flex: 1 }}
          />
          <TextInput
            label="Gateway URL"
            placeholder="http://192.168.1.100/send"
            value={newUrl}
            onChange={(e) => setNewUrl(e.currentTarget.value)}
            style={{ flex: 2 }}
          />
          <Button leftSection={<IconPlus size={16} />} onClick={handleAdd}>Add</Button>
        </Group>
      </Card>

      <Card withBorder padding="lg" radius="md">
        <Title order={2} mb="md">Configured Sectors</Title>
        <Table highlightOnHover>
          <Table.Thead>
            <Table.Tr>
              <Table.Th>Name</Table.Th>
              <Table.Th>Gateway URL</Table.Th>
              <Table.Th>Actions</Table.Th>
            </Table.Tr>
          </Table.Thead>
          <Table.Tbody>{rows}</Table.Tbody>
        </Table>
      </Card>
    </Stack>
  );
};
