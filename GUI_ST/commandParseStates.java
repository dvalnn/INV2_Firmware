enum ParseState {
  START,
  CMD,
  ID,
  PAYLOAD_LENGTH,
  PAYLOAD,
  CRC1,
  CRC2,
};
