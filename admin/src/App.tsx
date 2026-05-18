import { AppShell, Tabs, Container, Title, Group, Button } from '@mantine/core';
import { IconUsers, IconBroadcast, IconMapPin, IconAlertTriangle } from '@tabler/icons-react';
import { UserList } from './components/UserList';
import { SectorManager } from './components/SectorManager';
import { Messaging } from './components/Messaging';
import api from './api';
import { notifications } from '@mantine/notifications';

function App() {
  const handleUnlinkAll = async () => {
    if (!window.confirm('DANGER! This will unlink ALL users and badges. Proceed?')) return;
    try {
      await api.post('/badges/unlink-all');
      notifications.show({ title: 'Success', message: 'All badges unlinked', color: 'green' });
      window.location.reload();
    } catch (error) {
      notifications.show({ title: 'Error', message: 'Failed to unlink badges', color: 'red' });
    }
  };

  return (
    <AppShell
      header={{ height: 60 }}
      padding="md"
    >
      <AppShell.Header>
        <Container size="lg" h="100%">
          <Group justify="space-between" h="100%">
            <Title order={3} c="blue">Badge Admin Panel</Title>
            <Button 
              variant="light" 
              color="red" 
              leftSection={<IconAlertTriangle size={16} />}
              onClick={handleUnlinkAll}
            >
              End Event (Unlink All)
            </Button>
          </Group>
        </Container>
      </AppShell.Header>

      <AppShell.Main bg="gray.0">
        <Container size="lg">
          <Tabs defaultValue="messaging" variant="pills" radius="md">
            <Tabs.List mb="lg">
              <Tabs.Tab value="users" leftSection={<IconUsers size={16} />}>
                Users
              </Tabs.Tab>
              <Tabs.Tab value="sectors" leftSection={<IconMapPin size={16} />}>
                Sectors (Gateways)
              </Tabs.Tab>
              <Tabs.Tab value="messaging" leftSection={<IconBroadcast size={16} />}>
                Messaging
              </Tabs.Tab>
            </Tabs.List>

            <Tabs.Panel value="users">
              <UserList />
            </Tabs.Panel>

            <Tabs.Panel value="sectors">
              <SectorManager />
            </Tabs.Panel>

            <Tabs.Panel value="messaging">
              <Messaging />
            </Tabs.Panel>
          </Tabs>
        </Container>
      </AppShell.Main>
    </AppShell>
  );
}

export default App;
