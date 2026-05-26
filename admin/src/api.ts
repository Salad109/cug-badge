import axios from 'axios';

const api = axios.create({
  baseURL: import.meta.env.VITE_API_URL || 'http://localhost:8080',
});

export default api;

export interface User {
  id: number;
  name: string;
  surname: string;
  nickname: string;
  eMail: string;
  macAddress: string;
  role: 'USER' | 'VIP' | 'SPEAKER' | 'ADMIN' | 'STAFF';
  groups?: Group[];
}

export interface Sector {
  id: number;
  name: string;
  gatewayUrl: string;
}

export interface SectorRequest {
  name: string;
  gatewayUrl: string;
}

export interface Group {
  id: number;
  name: string;
  description: string;
  members?: User[];
}

export interface GroupRequest {
  name: string;
  description: string;
}

export interface MessageRequest {
  targetType: 'BROADCAST' | 'SECTOR' | 'MAC' | 'GROUP';
  targetId?: string;
  content: string;
  category: 'INFO' | 'WARNING' | 'SPONSOR' | 'ALERT';
}
