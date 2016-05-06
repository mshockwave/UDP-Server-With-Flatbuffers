// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_REMOVEREQ_FBS_ACCOUNT_H_
#define FLATBUFFERS_GENERATED_REMOVEREQ_FBS_ACCOUNT_H_

#include "flatbuffers/flatbuffers.h"

#include "types_generated.h"

namespace fbs {
struct Session;
struct GeneralResponse;
}  // namespace fbs

namespace fbs {
namespace account {

struct RemoveAccountRequest;

struct RemoveAccountRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  const fbs::Session *session() const { return GetPointer<const fbs::Session *>(4); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 4 /* session */) &&
           verifier.VerifyTable(session()) &&
           verifier.EndTable();
  }
};

struct RemoveAccountRequestBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_session(flatbuffers::Offset<fbs::Session> session) { fbb_.AddOffset(4, session); }
  RemoveAccountRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  RemoveAccountRequestBuilder &operator=(const RemoveAccountRequestBuilder &);
  flatbuffers::Offset<RemoveAccountRequest> Finish() {
    auto o = flatbuffers::Offset<RemoveAccountRequest>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<RemoveAccountRequest> CreateRemoveAccountRequest(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<fbs::Session> session = 0) {
  RemoveAccountRequestBuilder builder_(_fbb);
  builder_.add_session(session);
  return builder_.Finish();
}

inline const fbs::account::RemoveAccountRequest *GetRemoveAccountRequest(const void *buf) { return flatbuffers::GetRoot<fbs::account::RemoveAccountRequest>(buf); }

inline bool VerifyRemoveAccountRequestBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<fbs::account::RemoveAccountRequest>(); }

inline void FinishRemoveAccountRequestBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<fbs::account::RemoveAccountRequest> root) { fbb.Finish(root); }

}  // namespace account
}  // namespace fbs

#endif  // FLATBUFFERS_GENERATED_REMOVEREQ_FBS_ACCOUNT_H_
