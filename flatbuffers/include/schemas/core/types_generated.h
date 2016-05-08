// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_TYPES_FBS_H_
#define FLATBUFFERS_GENERATED_TYPES_FBS_H_

#include "flatbuffers/flatbuffers.h"


namespace fbs {

struct Session;
struct GeneralResponse;

enum Status {
  Status_OK = 0,
  Status_AUTH_ERROR = 1,
  Status_USER_EXIST = 2,
  Status_PAYLOAD_FORMAT_INVALID = 3,
  Status_REGISTER_INFO_INVALID = 4,
  Status_INVALID_REQUEST_ARGUMENT = 5,
  Status_UNKNOWN_ERROR = 6
};

inline const char **EnumNamesStatus() {
  static const char *names[] = { "OK", "AUTH_ERROR", "USER_EXIST", "PAYLOAD_FORMAT_INVALID", "REGISTER_INFO_INVALID", "INVALID_REQUEST_ARGUMENT", "UNKNOWN_ERROR", nullptr };
  return names;
}

inline const char *EnumNameStatus(Status e) { return EnumNamesStatus()[static_cast<int>(e)]; }

struct Session FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  const flatbuffers::String *token() const { return GetPointer<const flatbuffers::String *>(4); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 4 /* token */) &&
           verifier.Verify(token()) &&
           verifier.EndTable();
  }
};

struct SessionBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_token(flatbuffers::Offset<flatbuffers::String> token) { fbb_.AddOffset(4, token); }
  SessionBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  SessionBuilder &operator=(const SessionBuilder &);
  flatbuffers::Offset<Session> Finish() {
    auto o = flatbuffers::Offset<Session>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<Session> CreateSession(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::String> token = 0) {
  SessionBuilder builder_(_fbb);
  builder_.add_token(token);
  return builder_.Finish();
}

struct GeneralResponse FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  const Session *session() const { return GetPointer<const Session *>(4); }
  Status status_code() const { return static_cast<Status>(GetField<int8_t>(6, 0)); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 4 /* session */) &&
           verifier.VerifyTable(session()) &&
           VerifyField<int8_t>(verifier, 6 /* status_code */) &&
           verifier.EndTable();
  }
};

struct GeneralResponseBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_session(flatbuffers::Offset<Session> session) { fbb_.AddOffset(4, session); }
  void add_status_code(Status status_code) { fbb_.AddElement<int8_t>(6, static_cast<int8_t>(status_code), 0); }
  GeneralResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  GeneralResponseBuilder &operator=(const GeneralResponseBuilder &);
  flatbuffers::Offset<GeneralResponse> Finish() {
    auto o = flatbuffers::Offset<GeneralResponse>(fbb_.EndTable(start_, 2));
    return o;
  }
};

inline flatbuffers::Offset<GeneralResponse> CreateGeneralResponse(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<Session> session = 0,
   Status status_code = Status_OK) {
  GeneralResponseBuilder builder_(_fbb);
  builder_.add_session(session);
  builder_.add_status_code(status_code);
  return builder_.Finish();
}

inline const fbs::GeneralResponse *GetGeneralResponse(const void *buf) { return flatbuffers::GetRoot<fbs::GeneralResponse>(buf); }

inline bool VerifyGeneralResponseBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<fbs::GeneralResponse>(); }

inline void FinishGeneralResponseBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<fbs::GeneralResponse> root) { fbb.Finish(root); }

}  // namespace fbs

#endif  // FLATBUFFERS_GENERATED_TYPES_FBS_H_
