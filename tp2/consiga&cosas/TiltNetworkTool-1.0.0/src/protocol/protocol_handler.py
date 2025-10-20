from typing import List, Dict, Any
import logging
import re

class ProtocolHandler:
    """
    Clase intermedia para manejar el protocolo de comunicación serial.

    Qué se debe implementar:
      - Framing y parseo en on_bytes(): acumular bytes, detectar fin de mensaje,
        validar y convertir a una estructura uniforme para la GUI según lo especificado.
      - Construcción de mensajes salientes en build_led_command().
    """
    def __init__(self) -> None:
        # Buffer de recepción por si llegan fragmentos
        self._rxbuf = bytearray()
        # Regex flexibles para soportar variantes de formato
        # 1) A,<st>,<axis>,<val>   2) <st>,<axis>,<val>   3) ANG,<st>,<axis_name>,<val>
        self._re_line_generic = re.compile(
            r"""^\s*(?:A,|ANG,)?\s*
                 (?P<st>\d+)\s*[,;]\s*
                 (?P<ax>(?:[RCOrco]|[012]|ROLL|PITCH|YAW|roll|pitch|yaw))\s*[,;]\s*
                 (?P<val>[+-]?(?:\d+(?:\.\d+)?|\.\d+))\s*$""",
            re.VERBOSE
        )
        logging.info("[ProtocolHandler] Inicializado. Listo para recibir bytes del puerto serie.")

    def _axis_to_index(self, token: str) -> int:
        """Mapea R/C/O, 0/1/2 o nombres a {0:roll,1:pitch,2:yaw}."""
        t = token.strip().lower()
        if t in ("0", "r", "roll"):
            return 0
        if t in ("1", "c", "pitch"):
            return 1
        if t in ("2", "o", "yaw"):
            return 2
        raise ValueError(f"Eje inválido: {token}")

    def on_bytes(self, data: bytes) -> List[Dict[str, Any]]:
        """
        Recibe bytes crudos desde el puerto serie y devuelve a la GUI una lista de mensajes parseados.

        Debe devolver: lista de mensajes. Cada mensaje es un dict con:
          - 'station_index': int (0..N-1)
          - 'angle': int en {0: roll, 1: pitch, 2: yaw}
          - 'value': float|int

        Retorna [] si no hay líneas completas (terminadas en '\n').
        Soporta separadores ',' o ';' y mayúsc/minúsc para el eje.
        """
        logging.debug(f"[ProtocolHandler] RX chunk: {data!r}")
        out: List[Dict[str, Any]] = []

        # Acumular y procesar por líneas (framing por '\n')
        self._rxbuf.extend(data)
        while True:
            nl = self._rxbuf.find(b"\n")
            if nl < 0:
                break  # no hay línea completa aún

            # Tomar una línea completa (y tolerar '\r\n')
            line = self._rxbuf[:nl].decode(errors="ignore").strip("\r")
            # Consumir esa línea + '\n'
            del self._rxbuf[:nl+1]

            if not line.strip():
                continue  # línea vacía

            logging.debug(f"[ProtocolHandler] RX line: {line}")

            # Normalizar posibles prefijos (A, / ANG,) si vinieron
            if line.upper().startswith("A,") or line.upper().startswith("ANG,"):
                # ya está contemplado por el regex con prefijo opcional
                pass

            m = self._re_line_generic.match(line)
            if not m:
                logging.warning(f"[ProtocolHandler] Línea ignorada (formato no reconocido): {line}")
                continue

            try:
                st = int(m.group("st"))
                angle_idx = self._axis_to_index(m.group("ax"))
                # parsear valor como float (si es entero, luego se verá como 12.0; es OK para GUI)
                val_str = m.group("val")
                value = float(val_str)
                # Si es entero exacto, devolver int (por prolijidad)
                if value.is_integer():
                    value_out: Any = int(value)
                else:
                    value_out = value

                msg = {
                    "station_index": st,
                    "angle": angle_idx,
                    "value": value_out,
                }
                out.append(msg)
                logging.info(f"[ProtocolHandler] Parsed: {msg}")
            except Exception as e:
                logging.error(f"[ProtocolHandler] Error parseando línea '{line}': {e}")

        return out

    def build_led_command(self, station_index: int, r: bool, g: bool, b: bool) -> bytes:
        """
        Construye los bytes a enviar por serial para comandar LEDs de una estación.

        Parámetros:
          - station_index: int (0..N-1)
          - r, g, b: bools que indican encendido de cada color

        Devuelve:
          - bytes listos para write() del puerto serie, con formato:
            b'LED,<station>,<R>,<G>,<B>\\n'   (R|G|B en {0,1})
        """
        logging.info(f"[ProtocolHandler] Build LED cmd -> station={station_index}, R={r}, G={g}, B={b}")
        R = 1 if r else 0
        G = 1 if g else 0
        B = 1 if b else 0
        line = f"LED,{int(station_index)},{R},{G},{B}\n"
        logging.debug(f"[ProtocolHandler] TX line: {line.strip()}")
        return line.encode("ascii", errors="ignore")
