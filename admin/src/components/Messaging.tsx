import { useState, useEffect } from 'react';
import { Button, Textarea, Select, Title, Card, Stack, TextInput, SegmentedControl, Text } from '@mantine/core';
import { IconSend } from '@tabler/icons-react';
import api from '../api';
import type { Sector } from '../api';
import { notifications } from '@mantine/notifications';

export const Messaging = () => {
  const [targetType, setTargetType] = useState<string>('BROADCAST');
  const [targetId, setTargetId] = useState<string>('');
  const [content, setContent] = useState('');
  const [category, setCategory] = useState<string>('INFO');
  const [sectors, setSectors] = useState<Sector[]>([]);

  useEffect(() => {
    const fetchSectors = async () => {
      try {
        const response = await api.get('/sectors');
        setSectors(response.data);
      } catch (e) {}
    };
    fetchSectors();
  }, []);

  const handleSend = async () => {
    if (!content) return;
    try {
      await api.post('/admin/messages', {
        targetType,
        targetId: targetType === 'BROADCAST' ? undefined : targetId,
        content,
        category,
      });
      notifications.show({ title: 'Success', message: 'Message sent to queue', color: 'green' });
      setContent('');
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to send message', color: 'red' });
    }
  };

  return (
    <Card withBorder padding="lg" radius="md">
      <Title order={2} mb="md">Send Message to Badges</Title>
      <Stack>
        <Text size="sm" fw={500}>Target Audience</Text>
        <SegmentedControl
          value={targetType}
          onChange={setTargetType}
          data={[
            { label: 'Broadcast', value: 'BROADCAST' },
            { label: 'Sector', value: 'SECTOR' },
            { label: 'Specific MAC', value: 'MAC' },
          ]}
        />

        {targetType === 'SECTOR' && (
          <Select
            label="Select Sector"
            placeholder="Choose a sector"
            data={sectors.map(s => ({ value: s.id.toString(), label: s.name }))}
            value={targetId}
            onChange={(val) => setTargetId(val || '')}
          />
        )}

        {targetType === 'MAC' && (
          <TextInput
            label="Badge MAC Address"
            placeholder="AA:BB:CC:DD:EE:FF"
            value={targetId}
            onChange={(e) => setTargetId(e.currentTarget.value)}
          />
        )}

        <Select
          label="Message Category"
          data={['INFO', 'WARNING', 'SPONSOR', 'ALERT']}
          value={category}
          onChange={(val) => setCategory(val || 'INFO')}
        />

        <Textarea
          label="Message Content"
          placeholder="Type your message here (max 150 chars)..."
          maxLength={150}
          minRows={3}
          value={content}
          onChange={(e) => setContent(e.currentTarget.value)}
        />
        <Text size="xs" c="dimmed" ta="right">{content.length}/150</Text>

        <Button 
          leftSection={<IconSend size={16} />} 
          size="md" 
          onClick={handleSend}
          disabled={!content || (targetType !== 'BROADCAST' && !targetId)}
        >
          Send Message
        </Button>
      </Stack>
    </Card>
  );
};
