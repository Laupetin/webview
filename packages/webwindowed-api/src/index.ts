export type WebwindowedEventMethods<
  EventMap extends Record<string, unknown> = {},
> = {
  addEventListener<K extends keyof EventMap>(
    eventKey: K,
    callback: (payload: EventMap[K]) => void,
  ): void;
  removeEventListener<K extends keyof EventMap>(
    eventKey: K,
    callback: (payload: EventMap[K]) => void,
  ): boolean;
};

export function getBinds<Binds extends Record<string, unknown> = {}>(): Binds {
  const windowWithWebwindowedExtensions = window as typeof window & {
    webwindowedBinds: Binds;
  };

  return windowWithWebwindowedExtensions.webwindowedBinds;
}

export function getEvents<
  EventMap extends Record<string, unknown> = {},
>(): WebwindowedEventMethods<EventMap> {
  const windowWithWebwindowedExtensions = window as typeof window & {
    webwindowedAddEventListener: WebwindowedEventMethods["addEventListener"];
    webwindowedRemoveEventListener: WebwindowedEventMethods["removeEventListener"];
  };

  return {
    addEventListener:
      windowWithWebwindowedExtensions.webwindowedAddEventListener,
    removeEventListener:
      windowWithWebwindowedExtensions.webwindowedRemoveEventListener,
  };
}
