
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clang/ARCMigrate/ARCMTActions.h"
#include "clang/ARCMigrate/ARCMT.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;
using namespace arcmt;

bool CheckAction::BeginInvocation(CompilerInstance &CI) {
  if (arcmt::checkForManualIssues(CI.getInvocation(), getCurrentInput(),
                                  CI.getDiagnostics().getClient()))
    return false; // errors, stop the action.

  // We only want to see warnings reported from arcmt::checkForManualIssues.
  CI.getDiagnostics().setIgnoreAllWarnings(true);
  return true;
}

CheckAction::CheckAction(FrontendAction *WrappedAction)
  : WrapperFrontendAction(WrappedAction) {}

bool ModifyAction::BeginInvocation(CompilerInstance &CI) {
  return !arcmt::applyTransformations(CI.getInvocation(), getCurrentInput(),
                                      CI.getDiagnostics().getClient());
}

ModifyAction::ModifyAction(FrontendAction *WrappedAction)
  : WrapperFrontendAction(WrappedAction) {}

bool MigrateAction::BeginInvocation(CompilerInstance &CI) {
  if (arcmt::migrateWithTemporaryFiles(CI.getInvocation(),
                                       getCurrentInput(),
                                       CI.getDiagnostics().getClient(),
                                       MigrateDir,
                                       EmitPremigrationARCErros,
                                       PlistOut))
    return false; // errors, stop the action.

  // We only want to see diagnostics emitted by migrateWithTemporaryFiles.
  CI.getDiagnostics().setIgnoreAllWarnings(true);
  return true;
}

MigrateAction::MigrateAction(FrontendAction *WrappedAction,
                             StringRef migrateDir,
                             StringRef plistOut,
                             bool emitPremigrationARCErrors)
  : WrapperFrontendAction(WrappedAction), MigrateDir(migrateDir),
    PlistOut(plistOut), EmitPremigrationARCErros(emitPremigrationARCErrors) {
  if (MigrateDir.empty())
    MigrateDir = "."; // user current directory if none is given.
}