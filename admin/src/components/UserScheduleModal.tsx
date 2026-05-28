import { useState, useEffect } from 'react';
import { Modal, Table, Button, Group, Stack, Text, MultiSelect, ActionIcon } from '@mantine/core';
import { IconTrash, IconBroadcast } from '@tabler/icons-react';
import api from '../api';
import type { User, AgendaItem } from '../api';
import { notifications } from '@mantine/notifications';

interface UserScheduleModalProps {
  opened: boolean;
  onClose: () => void;
  user: User | null;
}

export const UserScheduleModal = ({ opened, onClose, user }: UserScheduleModalProps) => {
  const [userSchedule, setUserSchedule] = useState<AgendaItem[]>([]);
  const [allAgendaItems, setAllAgendaItems] = useState<AgendaItem[]>([]);
  const [selectedItemIds, setSelectedItemIds] = useState<string[]>([]);

  const fetchData = async () => {
    if (!user) return;
    try {
      const [scheduleRes, allItemsRes] = await Promise.all([
        api.get(`/users/${user.id}/schedule`),
        api.get('/agenda/items')
      ]);
      setUserSchedule(scheduleRes.data);
      setAllAgendaItems(allItemsRes.data);
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to fetch schedule data', color: 'red' });
    }
  };

  useEffect(() => {
    if (opened && user) {
      fetchData();
    }
  }, [opened, user]);

  const handleAdd = async () => {
    if (!user || selectedItemIds.length === 0) return;
    try {
      await Promise.all(selectedItemIds.map(itemId => 
        api.post(`/users/${user.id}/schedule/${itemId}`)
      ));
      notifications.show({ title: 'Success', message: 'Schedule updated', color: 'green' });
      setSelectedItemIds([]);
      fetchData();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to add items to schedule', color: 'red' });
    }
  };

  const handleDelete = async (itemId: number) => {
    if (!user) return;
    try {
      await api.delete(`/users/${user.id}/schedule/${itemId}`);
      notifications.show({ title: 'Success', message: 'Item removed', color: 'green' });
      fetchData();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to remove item', color: 'red' });
    }
  };

  const handlePush = async () => {
    if (!user) return;
    try {
      await api.post(`/users/${user.id}/schedule/push`);
      notifications.show({ title: 'Pushed', message: 'Schedule sync command sent via LoRa', color: 'blue' });
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to push schedule', color: 'red' });
    }
  };

  const selectData = allAgendaItems
    .filter(item => !userSchedule.find(us => us.id === item.id))
    .map(item => ({ value: item.id.toString(), label: `${item.title} (${item.location})` }));

  const rows = userSchedule.map((item) => (
    <Table.Tr key={item.id}>
      <Table.Td>{item.title}</Table.Td>
      <Table.Td>{item.location}</Table.Td>
      <Table.Td>
        <ActionIcon color="red" variant="subtle" onClick={() => handleDelete(item.id)}>
          <IconTrash size={16} />
        </ActionIcon>
      </Table.Td>
    </Table.Tr>
  ));

  return (
    <Modal opened={opened} onClose={onClose} title={`Schedule for ${user?.nickname}`} size="lg">
      <Stack>
        <Group align="flex-end">
          <MultiSelect
            label="Add Agenda Items"
            placeholder="Select items"
            data={selectData}
            value={selectedItemIds}
            onChange={setSelectedItemIds}
            searchable
            style={{ flex: 1 }}
          />
          <Button onClick={handleAdd} disabled={selectedItemIds.length === 0}>Add</Button>
        </Group>

        <Text size="sm" fw={500} mt="md">Current Schedule:</Text>
        <Table highlightOnHover>
          <Table.Thead>
            <Table.Tr>
              <Table.Th>Title</Table.Th>
              <Table.Th>Location</Table.Th>
              <Table.Th>Actions</Table.Th>
            </Table.Tr>
          </Table.Thead>
          <Table.Tbody>{rows}</Table.Tbody>
        </Table>

        <Button 
          fullWidth 
          leftSection={<IconBroadcast size={16} />} 
          color="blue" 
          mt="xl"
          onClick={handlePush}
          disabled={!user?.macAddress}
        >
          Push to Badge (LoRa Sync)
        </Button>
      </Stack>
    </Modal>
  );
};
