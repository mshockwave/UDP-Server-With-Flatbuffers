// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_LOGOUTRESP_FBS_ACCOUNT_H_
#define FLATBUFFERS_GENERATED_LOGOUTRESP_FBS_ACCOUNT_H_

#include "flatbuffers/flatbuffers.h"

#include "types_generated.h"

namespace fbs {
struct Session;
struct GeneralResponse;
}  // namespace fbs

namespace fbs {
namespace account {

struct LogoutResponse;

struct LogoutResponse FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  fbs::Status status_code() const { return static_cast<fbs::Status>(GetField<int8_t>(4, 0)); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, 4 /* status_code */) &&
           verifier.EndTable();
  }
};

struct LogoutResponseBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_status_code(fbs::Status status_code) { fbb_.AddElement<int8_t>(4, static_cast<int8_t>(status_code), 0); }
  LogoutResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  LogoutResponseBuilder &operator=(const LogoutResponseBuilder &);
  flatbuffers::Offset<LogoutResponse> Finish() {
    auto o = flatbuffers::Offset<LogoutResponse>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<LogoutResponse> CreateLogoutResponse(flatbuffers::FlatBufferBuilder &_fbb,
   fbs::Status status_code = fbs::Status_OK) {
  LogoutResponseBuilder builder_(_fbb);
  builder_.add_status_code(status_code);
  return builder_.Finish();
}

inline const fbs::account::LogoutResponse *GetLogoutResponse(const void *buf) { return flatbuffers::GetRoot<fbs::account::LogoutResponse>(buf); }

inline bool VerifyLogoutResponseBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<fbs::account::LogoutResponse>(); }

inline void FinishLogoutResponseBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<fbs::account::LogoutResponse> root) { fbb.Finish(root); }

}  // namespace account
}  // namespace fbs

#endif  // FLATBUFFERS_GENERATED_LOGOUTRESP_FBS_ACCOUNT_H_
