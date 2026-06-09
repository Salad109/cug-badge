import { useState, useEffect } from 'react';
import { Modal, TextInput, Select, Button, Stack, Group } from '@mantine/core';
import type { User } from '../api';

interface UserModalProps {
  opened: boolean;
  onClose: () => void;
  onSubmit: (user: Partial<User>) => void;
  initialData?: User | null;
}

export const UserModal = ({ opened, onClose, onSubmit, initialData }: UserModalProps) => {
  const [formData, setFormData] = useState<Partial<User>>({
    name: '',
    surname: '',
    nickname: '',
    company: '',
    position: '',
    eMail: '',
    macAddress: '',
    role: 'USER',
  });

  useEffect(() => {
    if (initialData) {
      setFormData(initialData);
    } else {
      setFormData({
        name: '',
        surname: '',
        nickname: '',
        company: '',
        position: '',
        eMail: '',
        macAddress: '',
        role: 'USER',
      });
    }
  }, [initialData, opened]);

  return (
    <Modal opened={opened} onClose={onClose} title={initialData ? 'Edit User' : 'Add New User'}>
      <Stack>
        <TextInput
          label="Name"
          required
          value={formData.name}
          onChange={(e) => setFormData({ ...formData, name: e.target.value })}
        />
        <TextInput
          label="Surname"
          value={formData.surname}
          onChange={(e) => setFormData({ ...formData, surname: e.target.value })}
        />
        <TextInput
          label="Nickname"
          required
          value={formData.nickname}
          onChange={(e) => setFormData({ ...formData, nickname: e.target.value })}
        />
        <TextInput
          label="Company"
          value={formData.company}
          onChange={(e) => setFormData({ ...formData, company: e.target.value })}
        />
        <TextInput
          label="Position/Role"
          value={formData.position}
          onChange={(e) => setFormData({ ...formData, position: e.target.value })}
        />
        <TextInput
          label="Email"
          value={formData.eMail}
          onChange={(e) => setFormData({ ...formData, eMail: e.target.value })}
        />
        <TextInput
          label="MAC Address"
          placeholder="AA:BB:CC:DD:EE:FF"
          value={formData.macAddress}
          onChange={(e) => setFormData({ ...formData, macAddress: e.target.value })}
        />
        <Select
          label="Role"
          data={['USER', 'VIP', 'SPEAKER', 'ADMIN', 'STAFF']}
          value={formData.role}
          onChange={(val) => setFormData({ ...formData, role: val as any })}
        />
        <Group justify="flex-end" mt="md">
          <Button variant="subtle" onClick={onClose}>Cancel</Button>
          <Button onClick={() => onSubmit(formData)}>Save</Button>
        </Group>
      </Stack>
    </Modal>
  );
};
