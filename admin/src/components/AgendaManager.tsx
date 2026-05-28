import { useEffect, useState } from 'react';
import { Table, Button, TextInput, Group, ActionIcon, Title, Card, Stack, Checkbox, Textarea, Modal } from '@mantine/core';
import { IconTrash, IconPlus, IconEdit } from '@tabler/icons-react';
import api from '../api';
import type { AgendaItem, AgendaItemRequest } from '../api';
import { notifications } from '@mantine/notifications';
import { useDisclosure } from '@mantine/hooks';

export const AgendaManager = () => {
  const [items, setItems] = useState<AgendaItem[]>([]);
  const [opened, { open, close }] = useDisclosure(false);
  const [editingItem, setEditingItem] = useState<AgendaItem | null>(null);

  // Form state
  const [formData, setFormData] = useState<AgendaItemRequest>({
    title: '',
    description: '',
    location: '',
    startTime: '',
    endTime: '',
    track: '',
    speaker: '',
    isDefault: false
  });

  const fetchItems = async () => {
    try {
      const response = await api.get('/agenda/items');
      setItems(response.data);
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to fetch agenda items', color: 'red' });
    }
  };

  useEffect(() => {
    fetchItems();
  }, []);

  const handleOpenAdd = () => {
    setEditingItem(null);
    setFormData({
      title: '',
      description: '',
      location: '',
      startTime: '',
      endTime: '',
      track: '',
      speaker: '',
      isDefault: false
    });
    open();
  };

  const handleOpenEdit = (item: AgendaItem) => {
    setEditingItem(item);
    setFormData({
      title: item.title,
      description: item.description,
      location: item.location,
      startTime: item.startTime,
      endTime: item.endTime,
      track: item.track,
      speaker: item.speaker || '',
      isDefault: item.isDefault
    });
    open();
  };

  const handleSubmit = async () => {
    try {
      if (editingItem) {
        await api.put(`/agenda/items/${editingItem.id}`, formData);
        notifications.show({ title: 'Success', message: 'Agenda item updated', color: 'green' });
      } else {
        await api.post('/agenda/items', formData);
        notifications.show({ title: 'Success', message: 'Agenda item added', color: 'green' });
      }
      close();
      fetchItems();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to save agenda item', color: 'red' });
    }
  };

  const handleDelete = async (id: number) => {
    if (!confirm('Are you sure you want to delete this item?')) return;
    try {
      await api.delete(`/agenda/items/${id}`);
      notifications.show({ title: 'Success', message: 'Item deleted', color: 'green' });
      fetchItems();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to delete item', color: 'red' });
    }
  };

  const rows = items.map((item) => (
    <Table.Tr key={item.id}>
      <Table.Td>{item.title}</Table.Td>
      <Table.Td>{item.location}</Table.Td>
      <Table.Td>{new Date(item.startTime).toLocaleString()}</Table.Td>
      <Table.Td>{item.track}</Table.Td>
      <Table.Td>{item.isDefault ? 'Yes' : 'No'}</Table.Td>
      <Table.Td>
        <Group gap="xs">
          <ActionIcon color="blue" onClick={() => handleOpenEdit(item)}>
            <IconEdit size={16} />
          </ActionIcon>
          <ActionIcon color="red" onClick={() => handleDelete(item.id)}>
            <IconTrash size={16} />
          </ActionIcon>
        </Group>
      </Table.Td>
    </Table.Tr>
  ));

  return (
    <Stack>
      <Card withBorder padding="lg" radius="md">
        <Group justify="space-between" mb="md">
          <Title order={2}>Agenda Management</Title>
          <Button leftSection={<IconPlus size={16} />} onClick={handleOpenAdd}>Add Item</Button>
        </Group>

        <Table highlightOnHover>
          <Table.Thead>
            <Table.Tr>
              <Table.Th>Title</Table.Th>
              <Table.Th>Location</Table.Th>
              <Table.Th>Time</Table.Th>
              <Table.Th>Track</Table.Th>
              <Table.Th>Default</Table.Th>
              <Table.Th>Actions</Table.Th>
            </Table.Tr>
          </Table.Thead>
          <Table.Tbody>{rows}</Table.Tbody>
        </Table>
      </Card>

      <Modal opened={opened} onClose={close} title={editingItem ? "Edit Agenda Item" : "Add Agenda Item"} size="lg">
        <Stack>
          <TextInput
            label="Title"
            required
            value={formData.title}
            onChange={(e) => setFormData({ ...formData, title: e.currentTarget.value })}
          />
          <Textarea
            label="Description"
            value={formData.description}
            onChange={(e) => setFormData({ ...formData, description: e.currentTarget.value })}
          />
          <Group grow>
            <TextInput
              label="Location"
              value={formData.location}
              onChange={(e) => setFormData({ ...formData, location: e.currentTarget.value })}
            />
            <TextInput
              label="Track"
              value={formData.track}
              onChange={(e) => setFormData({ ...formData, track: e.currentTarget.value })}
            />
          </Group>
          <Group grow>
            <TextInput
              label="Start Time"
              type="datetime-local"
              required
              value={formData.startTime.substring(0, 16)}
              onChange={(e) => setFormData({ ...formData, startTime: e.currentTarget.value })}
            />
            <TextInput
              label="End Time"
              type="datetime-local"
              required
              value={formData.endTime.substring(0, 16)}
              onChange={(e) => setFormData({ ...formData, endTime: e.currentTarget.value })}
            />
          </Group>
          <TextInput
            label="Speaker Name"
            value={formData.speaker}
            onChange={(e) => setFormData({ ...formData, speaker: e.currentTarget.value })}
          />
          <Checkbox
            label="Is Default (Auto-assigned to new users)"
            checked={formData.isDefault}
            onChange={(e) => setFormData({ ...formData, isDefault: e.currentTarget.checked })}
          />
          <Button onClick={handleSubmit}>{editingItem ? "Update" : "Create"}</Button>
        </Stack>
      </Modal>
    </Stack>
  );
};
